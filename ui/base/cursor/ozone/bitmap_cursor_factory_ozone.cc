// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/base/cursor/ozone/bitmap_cursor_factory_ozone.h"

#include "base/logging.h"
#include "third_party/skia/include/core/SkBitmap.h"

#if defined(USE_AURA)
#include "ui/base/cursor/cursors_aura.h"
#endif // USE_AURA

namespace ui {

namespace {

BitmapCursorOzone* ToBitmapCursorOzone(PlatformCursor cursor) {
  return static_cast<BitmapCursorOzone*>(cursor);
}

PlatformCursor ToPlatformCursor(BitmapCursorOzone* cursor) {
  return static_cast<PlatformCursor>(cursor);
}

scoped_refptr<BitmapCursorOzone> CreateDefaultBitmapCursor(int type) {
  SkBitmap bitmap;
  gfx::Point hotspot;

#if defined(USE_AURA)
  if (GetCursorBitmap(type, &bitmap, &hotspot))
    return new BitmapCursorOzone(bitmap, hotspot);
#endif // USE_AURA

  return NULL;
}

}  // namespace

BitmapCursorOzone::BitmapCursorOzone(const SkBitmap& bitmap,
                                     const gfx::Point& hotspot)
    : hotspot_(hotspot), frame_delay_ms_(0) {
  bitmaps_.push_back(bitmap);
}

BitmapCursorOzone::BitmapCursorOzone(const std::vector<SkBitmap>& bitmaps,
                                     const gfx::Point& hotspot,
                                     int frame_delay_ms)
    : bitmaps_(bitmaps), hotspot_(hotspot), frame_delay_ms_(frame_delay_ms) {
  DCHECK_LT(0U, bitmaps.size());
  DCHECK_LE(0, frame_delay_ms);
}

BitmapCursorOzone::~BitmapCursorOzone() {
}

const gfx::Point& BitmapCursorOzone::hotspot() {
  return hotspot_;
}

const SkBitmap& BitmapCursorOzone::bitmap() {
  return bitmaps_[0];
}

const std::vector<SkBitmap>& BitmapCursorOzone::bitmaps() {
  return bitmaps_;
}

int BitmapCursorOzone::frame_delay_ms() {
  return frame_delay_ms_;
}

BitmapCursorFactoryOzone::BitmapCursorFactoryOzone() {}

BitmapCursorFactoryOzone::~BitmapCursorFactoryOzone() {}

// static
scoped_refptr<BitmapCursorOzone> BitmapCursorFactoryOzone::GetBitmapCursor(
    PlatformCursor platform_cursor) {
  return make_scoped_refptr(ToBitmapCursorOzone(platform_cursor));
}

PlatformCursor BitmapCursorFactoryOzone::GetDefaultCursor(int type) {
  return GetDefaultCursorInternal(type).get();
}

PlatformCursor BitmapCursorFactoryOzone::CreateImageCursor(
    const SkBitmap& bitmap,
    const gfx::Point& hotspot) {
  BitmapCursorOzone* cursor = new BitmapCursorOzone(bitmap, hotspot);
  cursor->AddRef();  // Balanced by UnrefImageCursor.
  return ToPlatformCursor(cursor);
}

PlatformCursor BitmapCursorFactoryOzone::CreateAnimatedCursor(
    const std::vector<SkBitmap>& bitmaps,
    const gfx::Point& hotspot,
    int frame_delay_ms) {
  DCHECK_LT(0U, bitmaps.size());
  BitmapCursorOzone* cursor =
      new BitmapCursorOzone(bitmaps, hotspot, frame_delay_ms);
  cursor->AddRef();  // Balanced by UnrefImageCursor.
  return ToPlatformCursor(cursor);
}

void BitmapCursorFactoryOzone::RefImageCursor(PlatformCursor cursor) {
  ToBitmapCursorOzone(cursor)->AddRef();
}

void BitmapCursorFactoryOzone::UnrefImageCursor(PlatformCursor cursor) {
  ToBitmapCursorOzone(cursor)->Release();
}

scoped_refptr<BitmapCursorOzone>
BitmapCursorFactoryOzone::GetDefaultCursorInternal(int type) {
  if (type == kCursorNone)
    return NULL;  // NULL is used for hidden cursor.

  if (!default_cursors_.count(type)) {
    // Create new image cursor from default aura bitmap for this type. We hold a
    // ref forever because clients do not do refcounting for default cursors.
    scoped_refptr<BitmapCursorOzone> cursor = CreateDefaultBitmapCursor(type);
    if (!cursor.get() && type != kCursorPointer)
      cursor = GetDefaultCursorInternal(kCursorPointer);
    DCHECK(cursor.get()) << "Failed to load default cursor bitmap";
    default_cursors_[type] = cursor;
  }

  // Returned owned default cursor for this type.
  return default_cursors_[type];
}

}  // namespace ui
