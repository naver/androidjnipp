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

#include "PassLocalRef.h"

namespace JNI {

template<typename T>
class WeakGlobalRef final {
public:
    WeakGlobalRef()
        : m_ref(0)
    {
    }
    WeakGlobalRef(ref_t oref)
        : m_ref(JNI::refWeakGlobal(oref))
    {
    }
    WeakGlobalRef(weak_t ref)
        : m_ref(ref)
    {
    }
    template<typename U> WeakGlobalRef(const PassLocalRef<U>& ref)
        : m_ref(JNI::refWeakGlobal(ref.get()))
    {
        ref.derefIfNotNull();
    }
    ~WeakGlobalRef()
    {
        derefIfNotNull();
    }

    void reset() { derefIfNotNull(); }

    bool isExpired() const
    {
        if (!m_ref)
            return true;

        if (JNI::isExpiredWeakGlobal(m_ref)) {
            JNI::derefWeakGlobal(m_ref);
            m_ref = 0;
            return true;
        }

        return false;
    }

    weak_t leak()
    {
        weak_t oref = 0;
        std::swap(oref, m_ref);
        return oref;
    }

    PassLocalRef<T> tryPromote() const
    {
        if (isExpired())
            return PassLocalRef<T>();

        return T::fromRef(JNI::refLocal(m_ref));
    }

    template<typename U> PassLocalRef<U> tryPromote() const
    {
        if (isExpired())
            return PassLocalRef<U>();

        return U::fromRef(JNI::refLocal(m_ref));
    }

    bool operator!() const { return isExpired(); }

    // This conversion operator allows implicit conversion to bool but not to other integer types.
    typedef weak_t (WeakGlobalRef::*UnspecifiedBoolType);
    operator UnspecifiedBoolType() const { return !isExpired(); }

    WeakGlobalRef& operator=(const PassLocalRef<T>&);
    template<typename U> WeakGlobalRef& operator=(const PassLocalRef<U>&);

    void swap(WeakGlobalRef&);

private:
    void derefIfNotNull()
    {
        if (m_ref) {
            weak_t oref = 0;
            std::swap(oref, m_ref);
            JNI::derefWeakGlobal(oref);
        }
    }

    mutable weak_t m_ref;
}; // class WeakGlobalRef

template<typename T> inline WeakGlobalRef<T>& WeakGlobalRef<T>::operator=(const PassLocalRef<T>& o)
{
    WeakGlobalRef ptr = o;
    swap(ptr);
    return *this;
}

template<typename T> template<typename U> inline WeakGlobalRef<T>& WeakGlobalRef<T>::operator=(const PassLocalRef<U>& o)
{
    WeakGlobalRef ptr = o;
    swap(ptr);
    return *this;
}

template<class T> inline void WeakGlobalRef<T>::swap(WeakGlobalRef& o)
{
    std::swap(m_ref, o.m_ref);
}

template<class T> inline void swap(WeakGlobalRef<T>& a, WeakGlobalRef<T>& b)
{
    a.swap(b);
}

} // namespace JNI
