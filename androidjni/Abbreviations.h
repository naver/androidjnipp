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

#if !defined(UNUSED_PARAM) && defined(_MSC_VER)
#define UNUSED_PARAM(variable) (void)&variable
#endif

#if !defined(UNUSED_PARAM)
#define UNUSED_PARAM(variable) (void)variable
#endif

#define FIELD_INTERFACE_CLASS_NAME(FieldName) GeneratedClassFor##FieldName

#define FIELD_INTERFACE(FieldName, NativeType) \
    class CLASS_EXPORT FIELD_INTERFACE_CLASS_NAME(FieldName) final { \
    public: \
        void set(NativeType); \
        std::decay<NativeType>::type get(); \
        FIELD_INTERFACE_CLASS_NAME(FieldName)(JNI::weak_t& ref) : m_ref(ref) { } \
        ~FIELD_INTERFACE_CLASS_NAME(FieldName)() { } \
    private: \
        JNI::weak_t& m_ref; \
    } FieldName

#define STATIC_FIELD_INTERFACE(FieldName, NativeType) \
    static class CLASS_EXPORT FIELD_INTERFACE_CLASS_NAME(FieldName) final { \
    public: \
        void set(NativeType); \
        std::decay<NativeType>::type get(); \
    } FieldName

#define ADD_TYPE_MAPPING(ManagedType, NativeType) \
    static inline ManagedType toManaged(NativeType value) { return (ManagedType)value; } \
    static inline NativeType toNative(ManagedType value) { return (NativeType)value; }
