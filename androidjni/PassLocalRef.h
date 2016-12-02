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

#include "NativeObject.h"

namespace JNI {

template<typename T>
class PassLocalRef;

enum AdoptRefTagType { AdoptRefTag };

template<typename T> PassLocalRef<T> adoptRef(ref_t, T*);

template<typename T>
class LocalRef;

template<typename T>
class GlobalRef;

template<typename T>
class WeakGlobalRef;

template<typename T>
class PassLocalRef final {
public:
    PassLocalRef()
        : m_ptr(nullptr), m_ref(0)
    {
    }
    PassLocalRef(std::nullptr_t)
        : m_ptr(nullptr), m_ref(0)
    {
    }
    PassLocalRef(T* ptr) // For already bound NativeObjects.
        : m_ptr(ptr)
        , m_ref(0)
    {
        refIfNotNull();
    }
    PassLocalRef(ref_t oref, T* ptr)
        : m_ptr(ptr)
        , m_ref(JNI::refLocal(oref))
    {
        refIfNotNull();
    }
    PassLocalRef(const PassLocalRef& ref)
        : m_ptr(ref.m_ptr)
        , m_ref(ref.leak())
    {
    }
    template<typename U> PassLocalRef(const PassLocalRef<U>& ref)
        : m_ptr(ref.template getPtr<T>())
        , m_ref(ref.leak())
    {
    }
    template<typename U> PassLocalRef(const LocalRef<U>& ref)
        : m_ptr(ref.template getPtr<T>())
        , m_ref(JNI::refLocal(ref.get()))
    {
        refIfNotNull();
    }
    template<typename U> PassLocalRef(const GlobalRef<U>& ref)
        : m_ptr(ref.template getPtr<T>())
        , m_ref(JNI::refLocal(ref.get()))
    {
        refIfNotNull();
    }
    ~PassLocalRef()
    {
        derefIfNotNull();
    }

    ref_t leak() const
    {
        ref_t ref = m_ref;
        m_ptr = nullptr;
        m_ref = 0;
        return ref;
    }

    ref_t get() const { return m_ref; }

    T* getPtr() const
    {
        if (!m_ptr)
            return nullptr;

        return JNI::getPtr<T>(m_ref, m_ptr);
    }
    template<typename U> U* getPtr() const { return JNI::getPtr<U>(m_ref, getPtr()); }

    T& operator*() const { return *getPtr(); }
    T* operator->() const { return getPtr(); }

    bool operator!() const { return !m_ref; }

    template<typename U> PassLocalRef<U> as()
    {
        T* optr = nullptr;
        ref_t oref = 0;
        std::swap(optr, m_ptr);
        std::swap(oref, m_ref);
        return adoptRef(oref, JNI::adoptPtr<U>(oref, optr));
    }

    // This conversion operator allows implicit conversion to bool but not to other integer types.
    typedef ref_t (PassLocalRef::*UnspecifiedBoolType);
    operator UnspecifiedBoolType() const { return m_ref ? &PassLocalRef::m_ref : nullptr; }

    friend PassLocalRef adoptRef<T>(ref_t, T*);

private:
    PassLocalRef(ref_t ref, T* ptr, AdoptRefTagType)
        : m_ptr(ptr)
        , m_ref(ref)
    {
    }

    PassLocalRef& operator=(const PassLocalRef&) = delete;

    void refIfNotNull() const
    {
        if (m_ptr) {
            m_ptr->ref();

            if (!m_ref)
                m_ref = m_ptr->refLocal();
        }
    }
    void derefIfNotNull() const
    {
        if (m_ref) {
            ref_t oref = 0;
            std::swap(oref, m_ref);
            JNI::derefLocal(oref);
        }
        if (m_ptr) {
            T* optr = nullptr;
            std::swap(optr, m_ptr);
            optr->deref();
        }
    }

    friend class LocalRef<T>;
    friend class GlobalRef<T>;
    friend class WeakGlobalRef<T>;

    template<typename U> friend class LocalRef;
    template<typename U> friend class GlobalRef;
    template<typename U> friend class WeakGlobalRef;

    mutable T* m_ptr;
    mutable ref_t m_ref;
}; // class PassLocalRef

template<typename T> inline PassLocalRef<T> adoptRef(ref_t ref, T* ptr)
{
    return PassLocalRef<T>(ref, ptr, AdoptRefTag);
}

template<typename T, typename U> inline PassLocalRef<T> static_pointer_cast(const PassLocalRef<U>& p)
{
    return PassLocalRef<T>(p.get(), static_cast<T*>(p.getPtr()));
}

} // namespace JNI
