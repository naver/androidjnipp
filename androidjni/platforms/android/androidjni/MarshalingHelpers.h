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

#include "PassArray.h"
#include "JavaVM.h"
#include <androidjni/JNIIncludes.h>

namespace JNI {

ADD_TYPE_MAPPING(jint, int32_t);
ADD_TYPE_MAPPING(jlong, int64_t);
ADD_TYPE_MAPPING(jshort, int16_t);
ADD_TYPE_MAPPING(jbyte, int8_t);
ADD_TYPE_MAPPING(jfloat, float);
ADD_TYPE_MAPPING(jdouble, double);
ADD_TYPE_MAPPING(jboolean, bool);

jstring toManaged(const std::string&);
std::string toNative(jstring);

template<typename T, typename U>
jobject toManaged(const PassLocalRef<U>& ref)
{
    return reinterpret_cast<jobject>(ref.leak());
}

template<typename T>
PassLocalRef<T> toNative(jobject ref)
{
    return (ref) ? T::fromRef(ref) : nullptr;
}

template<typename T>
inline jobject toManaged(PassLocalRef<AnyObject>& ref)
{
    return reinterpret_cast<jobject>(ref.leak());
}

template<typename T>
inline jobject toManaged(PassLocalRef<AnyObject>&& ref)
{
    return reinterpret_cast<jobject>(ref.leak());
}

template<> inline PassLocalRef<AnyObject> toNative(jobject ref)
{
    return adoptRef(ref, reinterpret_cast<AnyObject*>(0));
}

template<typename T>
inline jobject toManaged(PassArray<T> arrayObject)
{
    return reinterpret_cast<jobject>(arrayObject.leak());
}

inline jintArray toManaged(PassArray<int32_t> arrayObject)
{
    return reinterpret_cast<jintArray>(arrayObject.leak());
}

inline PassArray<int32_t> toNative(jintArray array)
{
    return PassArray<int32_t>(reinterpret_cast<ref_t>(array));
}

inline jshortArray toManaged(PassArray<int16_t> arrayObject)
{
    return reinterpret_cast<jshortArray>(arrayObject.leak());
}

inline PassArray<int16_t> toNative(jshortArray array)
{
    return PassArray<int16_t>(reinterpret_cast<ref_t>(array));
}

inline jbyteArray toManaged(PassArray<int8_t> arrayObject)
{
    return reinterpret_cast<jbyteArray>(arrayObject.leak());
}

inline PassArray<int8_t> toNative(jbyteArray array)
{
    return PassArray<int8_t>(reinterpret_cast<ref_t>(array));
}

inline jfloatArray toManaged(PassArray<float> arrayObject)
{
    return reinterpret_cast<jfloatArray>(arrayObject.leak());
}

inline PassArray<float> toNative(jfloatArray array)
{
    return PassArray<float>(reinterpret_cast<ref_t>(array));
}

inline jdoubleArray toManaged(PassArray<double> arrayObject)
{
    return reinterpret_cast<jdoubleArray>(arrayObject.leak());
}

inline PassArray<double> toNative(jdoubleArray array)
{
    return PassArray<double>(reinterpret_cast<ref_t>(array));
}

inline jstringArray toManaged(PassArray<std::string> arrayObject)
{
    return reinterpret_cast<jstringArray>(arrayObject.leak());
}

inline PassArray<std::string> toNative(jstringArray array)
{
    return PassArray<std::string>(reinterpret_cast<ref_t>(array));
}

template<typename T, typename U>
inline jobjectArray toManaged(PassArray<PassLocalRef<U>> arrayObject)
{
    return reinterpret_cast<jobjectArray>(arrayObject.leak());
}

}
