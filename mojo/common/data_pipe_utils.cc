// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "mojo/common/data_pipe_utils.h"

#include <stdio.h>

#include "base/files/file_path.h"
#include "base/files/file_util.h"
#include "base/files/scoped_file.h"
#include "base/message_loop/message_loop.h"
#include "base/task_runner_util.h"
#include "base/threading/platform_thread.h"

namespace mojo {
namespace common {
namespace {

bool BlockingCopyHelper(ScopedDataPipeConsumerHandle source,
    const base::Callback<size_t(const void*, uint32_t)>& write_bytes) {
  for (;;) {
    const void* buffer;
    uint32_t num_bytes;
    MojoResult result = BeginReadDataRaw(
        source.get(), &buffer, &num_bytes, MOJO_READ_DATA_FLAG_NONE);
    if (result == MOJO_RESULT_OK) {
      size_t bytes_written = write_bytes.Run(buffer, num_bytes);
      result = EndReadDataRaw(source.get(), num_bytes);
      if (bytes_written < num_bytes || result != MOJO_RESULT_OK)
        return false;
    } else if (result == MOJO_RESULT_SHOULD_WAIT) {
      result = Wait(source.get(),
                    MOJO_HANDLE_SIGNAL_READABLE,
                    MOJO_DEADLINE_INDEFINITE);
      if (result != MOJO_RESULT_OK) {
        // If the producer handle was closed, then treat as EOF.
        return result == MOJO_RESULT_FAILED_PRECONDITION;
      }
    } else if (result == MOJO_RESULT_FAILED_PRECONDITION) {
      // If the producer handle was closed, then treat as EOF.
      return true;
    } else {
      // Some other error occurred.
      break;
    }
  }

  return false;
}

size_t CopyToStringHelper(
    std::string* result, const void* buffer, uint32_t num_bytes) {
  result->append(static_cast<const char*>(buffer), num_bytes);
  return num_bytes;
}

size_t CopyToFileHelper(FILE* fp, const void* buffer, uint32_t num_bytes) {
  return fwrite(buffer, 1, num_bytes, fp);
}

// Sleep for as long as max_sleep_micros if the deadline hasn't been reached
// and the number of bytes read is still increasing. Returns true if sleep
// was actually called.
//
// This class is a substitute for being able to wait until N bytes are available
// from a data pipe. The MaybeSleep method is called when num_bytes_read are
// available but more are needed by the Peek operation. If a second
// Peek operation finds the same number of bytes after sleeping we assume
// that there's no point in trying again.
// TODO(hansmuller): this heuristic is weak. crbug.com/429377
class PeekSleeper {
 public:
  explicit PeekSleeper(MojoTimeTicks deadline)
      : deadline_(deadline),
        last_number_bytes_read_(0),
        max_sleep_micros_(1000 * 10) {
  }

  bool MaybeSleep(uint32 num_bytes_read) {
    if (num_bytes_read > 0 && last_number_bytes_read_ >= num_bytes_read)
      return false;
    last_number_bytes_read_ = num_bytes_read;

    MojoTimeTicks now(GetTimeTicksNow());
    if (now > deadline_)
      return false;

    MojoTimeTicks sleep_time = (deadline_ == 0)
        ? max_sleep_micros_
        : std::min<int64>(deadline_ - now, max_sleep_micros_);
    base::PlatformThread::Sleep(base::TimeDelta::FromMicroseconds(sleep_time));
    return true;
  }

 private:
  MojoTimeTicks deadline_; // 0 => MOJO_DEADLINE_INDEFINITE
  uint32 last_number_bytes_read_;
  const MojoTimeTicks max_sleep_micros_; // microseconds
};

enum PeekStatus { kSuccess, kFail, kKeepReading };
typedef const base::Callback<PeekStatus(const void*, uint32_t, std::string*)>&
    PeekFunc;

// When data is available on source, call peek_func and then either return true
// and value, continue waiting for enough data to satisfy peek_func, or fail
// and return false. Fail if the timeout is exceeded.
bool BlockingPeekHelper(DataPipeConsumerHandle source,
                        std::string* value,
                        MojoDeadline timeout,
                        PeekFunc peek_func) {
  DCHECK(value);
  value->clear();

  MojoTimeTicks deadline = (timeout == MOJO_DEADLINE_INDEFINITE) ? 0
      : 1 + GetTimeTicksNow() + static_cast<MojoTimeTicks>(timeout);
  PeekSleeper sleeper(deadline);
  MojoResult result = MOJO_RESULT_OK;

  while(result == MOJO_RESULT_OK) {
    const void* buffer;
    uint32_t num_bytes;
    result =
        BeginReadDataRaw(source, &buffer, &num_bytes, MOJO_READ_DATA_FLAG_NONE);

    if (result == MOJO_RESULT_OK) {
      PeekStatus status = peek_func.Run(buffer, num_bytes, value);
      if (EndReadDataRaw(source, 0) != MOJO_RESULT_OK)
        return false;
      switch (status) {
        case PeekStatus::kSuccess: return true;
        case PeekStatus::kFail: return false;
        case PeekStatus::kKeepReading: break;
      }
      if (!sleeper.MaybeSleep(num_bytes))
        return false;
    } else if (result == MOJO_RESULT_SHOULD_WAIT) {
      MojoTimeTicks now(GetTimeTicksNow());
      if (timeout == MOJO_DEADLINE_INDEFINITE || now < deadline)
        result = Wait(source, MOJO_HANDLE_SIGNAL_READABLE, deadline - now);
    }
  }

  return false;
}

PeekStatus PeekLine(size_t max_line_length,
                    const void* buffer,
                    uint32 buffer_num_bytes,
                    std::string* line) {
  const char* p = static_cast<const char*>(buffer);
  size_t max_p_index = std::min<size_t>(buffer_num_bytes, max_line_length);
  for (size_t i = 0; i < max_p_index; i++) {
    if (p[i] == '\n') {
      *line = std::string(p, i + 1); // Include the trailing newline.
      return PeekStatus::kSuccess;
    }
  }
  return (buffer_num_bytes >= max_line_length)
      ? PeekStatus::kFail : PeekStatus::kKeepReading;
}

PeekStatus PeekNBytes(size_t bytes_length,
                      const void* buffer,
                      uint32 buffer_num_bytes,
                      std::string* bytes) {
  if (buffer_num_bytes >= bytes_length) {
    const char* p = static_cast<const char*>(buffer);
    *bytes = std::string(p, bytes_length);
    return PeekStatus::kSuccess;
  }
  return PeekStatus::kKeepReading;
}

} // namespace

bool BlockingPeekNBytes(DataPipeConsumerHandle source,
                        std::string* bytes,
                        size_t bytes_length,
                        MojoDeadline timeout) {
  PeekFunc peek_nbytes = base::Bind(PeekNBytes, bytes_length);
  return BlockingPeekHelper(source, bytes, timeout, peek_nbytes);
}

bool BlockingPeekLine(DataPipeConsumerHandle source,
                      std::string* line,
                      size_t max_line_length,
                      MojoDeadline timeout) {
  PeekFunc peek_line = base::Bind(PeekLine, max_line_length);
  return BlockingPeekHelper(source, line, timeout, peek_line);
}

// TODO(hansmuller): Add a max_size parameter.
bool BlockingCopyToString(ScopedDataPipeConsumerHandle source,
                          std::string* result) {
  CHECK(result);
  result->clear();
  return BlockingCopyHelper(
      source.Pass(), base::Bind(&CopyToStringHelper, result));
}

bool BlockingCopyToFile(ScopedDataPipeConsumerHandle source,
                        const base::FilePath& destination) {
  base::ScopedFILE fp(base::OpenFile(destination, "wb"));
  if (!fp)
    return false;
  return BlockingCopyHelper(
      source.Pass(), base::Bind(&CopyToFileHelper, fp.get()));
}

void CopyToFile(ScopedDataPipeConsumerHandle source,
                const base::FilePath& destination,
                base::TaskRunner* task_runner,
                const base::Callback<void(bool)>& callback) {
  base::PostTaskAndReplyWithResult(
      task_runner,
      FROM_HERE,
      base::Bind(&BlockingCopyToFile, base::Passed(&source), destination),
      callback);
}

}  // namespace common
}  // namespace mojo
