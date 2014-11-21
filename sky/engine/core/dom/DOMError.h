/*
 * Copyright (C) 2012 Google Inc. All Rights Reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef DOMError_h
#define DOMError_h

#include "sky/engine/bindings/core/v8/ScriptWrappable.h"
#include "sky/engine/core/dom/DOMException.h"
#include "sky/engine/core/dom/ExceptionCode.h"
#include "sky/engine/platform/heap/Handle.h"
#include "sky/engine/wtf/PassRefPtr.h"
#include "sky/engine/wtf/RefCounted.h"
#include "sky/engine/wtf/text/WTFString.h"

namespace blink {

class DOMError : public RefCounted<DOMError>, public ScriptWrappable {
    DEFINE_WRAPPERTYPEINFO();
public:
    static PassRefPtr<DOMError> create(const String& name)
    {
        return adoptRef(new DOMError(name));
    }
    static PassRefPtr<DOMError> create(const String& name, const String& message)
    {
        return adoptRef(new DOMError(name, message));
    }

    static PassRefPtr<DOMError> create(ExceptionCode ec)
    {
        return adoptRef(new DOMError(DOMException::getErrorName(ec), DOMException::getErrorMessage(ec)));
    }

    static PassRefPtr<DOMError> create(ExceptionCode ec, const String& message)
    {
        return adoptRef(new DOMError(DOMException::getErrorName(ec), message));
    }

    const String& name() const { return m_name; }
    const String& message() const { return m_message; }

protected:
    explicit DOMError(const String& name);
    DOMError(const String& name, const String& message);

private:
    const String m_name;
    const String m_message;
};

} // namespace blink

#endif // DOMError_h
