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

#include <androidjni/AnyObject.h>

namespace JNI {

template<typename T>
class PassLocalRef;

class JNI_EXPORT ObjectReference {
public:
    template<typename T> ObjectReference(const std::shared_ptr<T>& ptr)
        : m_localRefCount(1)
        , m_globalRefCount(0)
        , m_preventDeletion(0)
        , m_referencedPtr(std::static_pointer_cast<void>(ptr))
        , m_immutableReferencedPtrValue(m_referencedPtr.get())
    { }
    template<typename T> ObjectReference(std::shared_ptr<T>&& ptr)
        : m_localRefCount(1)
        , m_globalRefCount(0)
        , m_preventDeletion(0)
        , m_referencedPtr(std::static_pointer_cast<void>(ptr))
        , m_immutableReferencedPtrValue(m_referencedPtr.get())
    {
        ptr.reset();
    }
    virtual ~ObjectReference() = default;

    ref_t refLocal();
    void derefLocal(ref_t);

    ref_t refGlobal();
    void derefGlobal(ref_t);

    void* ptr() const { return m_immutableReferencedPtrValue; }
    void* safePtr() const { return m_referencedPtr.get(); }
    std::shared_ptr<void> sharePtr() const { return m_referencedPtr; }

    bool isExpired() const { return m_localRefCount.load() == 0 && m_globalRefCount.load() == 0 && m_dereferencedPtr.expired(); }
    void preventDeletion(bool f) { (f) ? ++m_preventDeletion : --m_preventDeletion; }
    void deleteIfPossible();

private:
    ObjectReference(const ObjectReference&) = delete;
    ObjectReference& operator=(const ObjectReference&) = delete;

    std::atomic<int32_t> m_localRefCount;
    std::atomic<int32_t> m_globalRefCount;
    std::shared_ptr<void> m_referencedPtr;
    std::weak_ptr<void> m_dereferencedPtr;
    void* const m_immutableReferencedPtrValue;
    std::atomic<int32_t> m_preventDeletion;
}; // class ObjectReference

template<typename T> inline T* getPtr(ref_t ref)
{
    return (ref) ? static_cast<T*>(reinterpret_cast<ObjectReference*>(ref)->ptr()) : nullptr;
}

inline std::shared_ptr<void> sharePtr(ref_t ref)
{
    return (ref) ? reinterpret_cast<ObjectReference*>(ref)->sharePtr() : nullptr;
}

} // namespace JNI
