// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SERVICES_KEYBOARD_NATIVE_KEY_LAYOUT_H_
#define SERVICES_KEYBOARD_NATIVE_KEY_LAYOUT_H_

#include <string>
#include <vector>

#include "base/callback.h"
#include "base/memory/weak_ptr.h"
#include "mojo/services/geometry/public/interfaces/geometry.mojom.h"
#include "ui/gfx/geometry/rect_f.h"

class SkCanvas;
class SkPaint;

namespace gfx {
class PointF;
}

namespace keyboard {

class Predictor;
class TextUpdateKey;

// Represents a drawable keyboard.
class KeyLayout {
 public:
  // All keys represented by this KeyBoard layout will implement the Key
  // interface.
  class Key {
   public:
    virtual ~Key() {}

    // Draws the Key to the SkCanvas with the given SkPaint to the given Rect.
    virtual void Draw(SkCanvas* canvas,
                      const SkPaint& paint,
                      const gfx::RectF& rect) = 0;

    // Converts the Key to its text representation.
    virtual const char* ToText() const = 0;

    // Indicate to the Key that a touch up has occurred on it.
    virtual void OnTouchUp() = 0;
  };

  class TextKey;

  KeyLayout();
  ~KeyLayout();

  // Sets the callback to call with the text of a key whenever it's pressed.
  void SetTextCallback(
      base::Callback<void(const std::string&)> on_text_callback);

  // Sets the callback to call whenever delete is pressed.
  void SetDeleteCallback(base::Callback<void()> on_delete_callback);

  void SetSuggestTextCallback(
      base::Callback<void(const std::string&)> on_suggest_text_callback);

  // Sets the dimensions the keyboard will draw itself into.
  void SetKeyArea(const gfx::RectF& key_area);

  // Draws the keyboard to the SkCanvas.
  void Draw(SkCanvas* canvas);

  // Returns the Key at the given Point.  Returns nullptr if no key is at that
  // Point.
  Key* GetKeyAtPoint(const gfx::PointF& point);

  // Indicate to the keyboard that a touch up has occurred at the given Point.
  void OnTouchUp(const gfx::PointF& touch_up);

  void SetPredictor(Predictor* predictor);

 private:
  // initializes the *_layout_ vectors.
  void InitLayouts();

  // initializes the *_key_map_ vectors.
  void InitKeyMaps();

  // A TextKey callback that does nothing.
  void OnKeyDoNothing(const TextKey& key);

  // A TextKey callback that calls on_text_callback_ with the text of the
  // TextKey.
  void OnKeyEmitText(const TextKey& key);

  // A TextKey callback that calls on_delete_callback_.
  void OnKeyDelete(const TextKey& key);

  void OnSuggestKeyEmitText(const TextUpdateKey& key);

  // A TextKey callback that switches the layout_ and key_map_ to upper case.
  void OnKeySwitchToUpperCase(const TextKey& key);

  // A TextKey callback that switches the layout_ and key_map_ to lower case.
  void OnKeySwitchToLowerCase(const TextKey& key);

  // A TextKey callback that switches the layout_ and key_map_ to symbols.
  void OnKeySwitchToSymbols(const TextKey& key);

  base::Callback<void(const std::string&)> on_text_callback_;
  base::Callback<void()> on_delete_callback_;
  base::Callback<void(const std::string&)> on_suggest_text_callback_;
  gfx::RectF key_area_;
  std::vector<std::vector<float>>* layout_;
  std::vector<std::vector<float>> letters_layout_;
  std::vector<std::vector<float>> symbols_layout_;
  std::vector<std::vector<Key*>>* key_map_;
  std::vector<std::vector<Key*>> lower_case_key_map_;
  std::vector<std::vector<Key*>> upper_case_key_map_;
  std::vector<std::vector<Key*>> symbols_key_map_;
  Predictor* predictor_;
  base::WeakPtrFactory<KeyLayout> weak_factory_;

  DISALLOW_COPY_AND_ASSIGN(KeyLayout);
};

}  // namespace keyboard

#endif  // SERVICES_KEYBOARD_NATIVE_KEY_LAYOUT_H_
