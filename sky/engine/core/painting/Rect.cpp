// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sky/engine/config.h"
#include "sky/engine/core/painting/Rect.h"

#include "sky/engine/tonic/dart_error.h"
#include "sky/engine/tonic/dart_state.h"
#include "base/logging.h"

namespace blink {

// Convert dart_rect._value[0...3] ==> SkRect.
Rect DartConverter<Rect, void>::FromArgumentsWithNullCheck(
    Dart_NativeArguments args,
    int index,
    Dart_Handle& exception) {
  Rect result;
  result.is_null = true;

  Dart_Handle dart_rect = Dart_GetNativeArgument(args, index);
  DCHECK(!LogIfError(dart_rect));

  Dart_Handle value = Dart_GetField(dart_rect,
                                    Dart_NewStringFromCString("_value"));
  if (Dart_IsNull(value))
    return result;

  Dart_TypedData_Type type;
  float* data = nullptr;
  intptr_t num_elements = 0;
  Dart_TypedDataAcquireData(
      value, &type, reinterpret_cast<void**>(&data), &num_elements);
  DCHECK(!LogIfError(value));
  ASSERT(type == Dart_TypedData_kFloat32 && num_elements == 4);

  SkScalar* dest[] = {
    &result.sk_rect.fLeft,
    &result.sk_rect.fTop,
    &result.sk_rect.fRight,
    &result.sk_rect.fBottom
  };
  for (intptr_t i = 0; i < 4; ++i)
    *dest[i] = data[i];

  Dart_TypedDataReleaseData(value);

  result.is_null = false;
  return result;
}

} // namespace blink