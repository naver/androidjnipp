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
class LocalRef;

template<typename T>
class GlobalRef final {
public:
    GlobalRef()
        : m_ptr(nullptr), m_ref(0)
    {
    }
    GlobalRef(std::nullptr_t)
        : m_ptr(nullptr), m_ref(0)
    {
    }
    GlobalRef(T* ptr) // For already bound NativeObjects.
        : m_ptr(ptr)
        , m_ref(0)
    {
        refIfNotNull();
    }
    GlobalRef(ref_t oref, T* ptr)
        : m_ptr(ptr)
        , m_ref(JNI::refGlobal(oref))
    {
        refIfNotNull();
    }
    GlobalRef(const GlobalRef& ref)
        : m_ptr(ref.getPtr())
        , m_ref(JNI::refGlobal(ref.get()))
    {
        refIfNotNull();
    }
    template<typename U> GlobalRef(const GlobalRef<U>& ref)
        : m_ptr(ref.template getPtr<T>())
        , m_ref(JNI::refGlobal(ref.get()))
    {
        refIfNotNull();
    }
    template<typename U> GlobalRef(const LocalRef<U>& ref)
        : m_ptr(ref.template getPtr<T>())
        , m_ref(JNI::refGlobal(ref.get()))
    {
        refIfNotNull();
    }
    template<typename U> GlobalRef(const PassLocalRef<U>& ref)
        : m_ptr(ref.template getPtr<T>())
        , m_ref(JNI::refGlobal(ref.get()))
    {
        refIfNotNull();
        ref.derefIfNotNull();
    }
    ~GlobalRef()
    {
        derefIfNotNull();
    }

    void reset() { derefIfNotNull(); }

    PassLocalRef<T> release()
    {
        PassLocalRef<T> ref(m_ref, m_ptr, AdoptRefTag);
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

    // This conversion operator allows implicit conversion to bool but not to other integer types.
    typedef ref_t (GlobalRef::*UnspecifiedBoolType);
    operator UnspecifiedBoolType() const { return m_ref ? &GlobalRef::m_ref : nullptr; }

    GlobalRef& operator=(const GlobalRef&);
    GlobalRef& operator=(T*);
    GlobalRef& operator=(const PassLocalRef<T>&);
    template<typename U> GlobalRef& operator=(const GlobalRef<U>&);
    template<typename U> GlobalRef& operator=(const LocalRef<U>&);
    template<typename U> GlobalRef& operator=(const PassLocalRef<U>&);

    void swap(GlobalRef&);

private:
    void refIfNotNull()
    {
        if (m_ptr) {
            m_ptr->ref();

            if (!m_ref)
                m_ref = m_ptr->refGlobal();
        }
    }
    void derefIfNotNull()
    {
        if (m_ref) {
            ref_t oref = 0;
            std::swap(oref, m_ref);
            JNI::derefGlobal(oref);
        }
        if (m_ptr) {
            T* optr = nullptr;
            std::swap(optr, m_ptr);
            optr->deref();
        }
    }

    T* m_ptr;
    ref_t m_ref;
}; // class GlobalRef

template<typename T> inline GlobalRef<T>& GlobalRef<T>::operator=(const GlobalRef& o)
{
    GlobalRef ptr = o;
    swap(ptr);
    return *this;
}
    
template<typename T> template<typename U> inline GlobalRef<T>& GlobalRef<T>::operator=(const GlobalRef<U>& o)
{
    GlobalRef ptr = o;
    swap(ptr);
    return *this;
}
    
template<typename T> template<typename U> inline GlobalRef<T>& GlobalRef<T>::operator=(const LocalRef<U>& o)
{
    GlobalRef ptr = o.getPtr();
    swap(ptr);
    return *this;
}

template<typename T> inline GlobalRef<T>& GlobalRef<T>::operator=(T* optr)
{
    GlobalRef ptr = optr;
    swap(ptr);
    return *this;
}

template<typename T> inline GlobalRef<T>& GlobalRef<T>::operator=(const PassLocalRef<T>& o)
{
    GlobalRef ptr = o;
    swap(ptr);
    return *this;
}

template<typename T> template<typename U> inline GlobalRef<T>& GlobalRef<T>::operator=(const PassLocalRef<U>& o)
{
    GlobalRef ptr = o;
    swap(ptr);
    return *this;
}

template<class T> inline void GlobalRef<T>::swap(GlobalRef& o)
{
    std::swap(m_ptr, o.m_ptr);
    std::swap(m_ref, o.m_ref);
}

template<class T> inline void swap(GlobalRef<T>& a, GlobalRef<T>& b)
{
    a.swap(b);
}

template<typename T, typename U> inline bool operator==(const GlobalRef<T>& a, const GlobalRef<U>& b)
{ 
    return a.getPtr() == b.getPtr(); 
}

template<typename T, typename U> inline bool operator==(const GlobalRef<T>& a, U* b)
{ 
    return a.getPtr() == b; 
}
    
template<typename T, typename U> inline bool operator==(T* a, const GlobalRef<U>& b) 
{
    return a == b.getPtr(); 
}
    
template<typename T, typename U> inline bool operator!=(const GlobalRef<T>& a, const GlobalRef<U>& b)
{ 
    return a.getPtr() != b.getPtr(); 
}

template<typename T, typename U> inline bool operator!=(const GlobalRef<T>& a, U* b)
{
    return a.getPtr() != b; 
}

template<typename T, typename U> inline bool operator!=(T* a, const GlobalRef<U>& b)
{ 
    return a != b.getPtr(); 
}
    
template<typename T, typename U> inline GlobalRef<T> static_pointer_cast(const GlobalRef<U>& p)
{ 
    return GlobalRef<T>(p.get(), static_cast<T*>(p.getPtr())); 
}

} // namespace JNI
