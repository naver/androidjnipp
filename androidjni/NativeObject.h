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

#include "AnyObject.h"

namespace JNI {

class JNI_EXPORT NativeObject : public AnyObject {
public:
    NativeObject()
        : m_refCount(1)
        , m_bind(JNI::refWeakGlobal(JNI::popLocalCallerObjectRef()))
    {
    }
    virtual ~NativeObject() { JNI::derefWeakGlobal(m_bind); }

    void ref() override { ++m_refCount; }
    void deref() override { --m_refCount; deleteIfPossible(); }

    ref_t refLocal() override { return JNI::refLocal(m_bind); }
    ref_t refGlobal() override { return JNI::refGlobal(m_bind); }

protected:
    NativeObject(const NativeObject&) = delete;
    NativeObject& operator=(const NativeObject&) = delete;

    void deleteIfPossible() { if (m_refCount > 0) return; delete this; }

    std::atomic<int32_t> m_refCount;
    weak_t m_bind;
}; // class NativeObject

template<typename T> inline T* getPtr(ref_t, NativeObject* ptr)
{
    return static_cast<T*>(ptr);
}

template<> inline AnyObject* getPtr<AnyObject>(ref_t, NativeObject* ptr)
{
    return reinterpret_cast<AnyObject*>(ptr);
}

template<typename T> inline T* adoptPtr(ref_t ref, NativeObject* ptr)
{
    return getPtr<T>(ref, ptr);
}

} // namespace JNI
