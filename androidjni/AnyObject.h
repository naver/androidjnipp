/*
 * Copyright (C) 2015 Naver Labs. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "ReferenceFunctions.h"

#include <atomic>

namespace JNI {

template<typename T> class LocalRef;

class JNI_EXPORT AnyObject {
public:
    AnyObject() = default;
    virtual ~AnyObject() = default;

    virtual void ref() { }
    virtual void deref() { }

    virtual ref_t refLocal() { return 0; }
    virtual ref_t refGlobal() { return 0; }

private:
    AnyObject(const AnyObject&) = delete;
    AnyObject& operator=(const AnyObject&) = delete;
}; // class AnyObject

template<typename T> inline T* getPtr(ref_t, AnyObject* ptr)
{
    return static_cast<T*>(ptr);
}

template<> inline AnyObject* getPtr<AnyObject>(ref_t, AnyObject* ptr)
{
    return ptr;
}

template<typename T> inline T* adoptPtr(ref_t ref, AnyObject* ptr)
{
    if (ptr)
        return getPtr<T>(ref, ptr);

    LocalRef<T> adoptedPtr = T::fromRef(JNI::refLocal(ref));
    adoptedPtr.getPtr()->ref();
    return adoptedPtr.getPtr();
}

} // namespace JNI
