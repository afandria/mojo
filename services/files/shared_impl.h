// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Shared implementation of things common between |DirectoryImpl| and
// |FileImpl|.

#ifndef SERVICES_FILES_SHARED_IMPL_H_
#define SERVICES_FILES_SHARED_IMPL_H_

#include "mojo/public/cpp/bindings/callback.h"
#include "mojo/services/files/public/interfaces/types.mojom.h"

namespace mojo {
namespace files {

// Stats the given FD (which must be valid), calling |callback| appropriately.
// The type in the |FileInformation| given to the callback will be assigned from
// |type|.
void StatFD(int fd,
            FileType type,
            const Callback<void(Error, FileInformationPtr)>& callback);

// Touches the given FD (which must be valid), calling |callback| appropriately.
void TouchFD(int fd,
             TimespecOrNowPtr atime,
             TimespecOrNowPtr mtime,
             const Callback<void(Error)>& callback);

}  // namespace files
}  // namespace mojo

#endif  // SERVICES_FILES_SHARED_IMPL_H_
