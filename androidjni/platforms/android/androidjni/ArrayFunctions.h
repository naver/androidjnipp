/*
 * Copyright (C) 2016 Naver Labs. All rights reserved.
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

#include <androidjni/PassLocalRef.h>

#include <vector>

namespace JNI {

void deleteArrayObject(ref_t arrayObject);
size_t getArrayObjectElementsCount(ref_t arrayObject);

template<typename T>
class ArrayFunctions {
public:
    static ref_t newArrayObject(const T* data, size_t count);
    static T* getArrayObjectElements(ref_t arrayObject);
    static void releaseArrayObjectElements(ref_t arrayObject, T* data, size_t count);
};

ref_t newIntArrayObject(const int32_t* data, size_t count);
int32_t* getIntArrayElements(ref_t arrayObject);
void releaseIntArrayElements(ref_t arrayObject, int32_t*, size_t count);

template<> class ArrayFunctions<int32_t> {
public:
    static ref_t newArrayObject(const int32_t* data, size_t count) { return newIntArrayObject(data, count); }
    static int32_t* getArrayObjectElements(ref_t arrayObject) { return getIntArrayElements(arrayObject); }
    static void releaseArrayObjectElements(ref_t arrayObject, int32_t* data, size_t count) { return releaseIntArrayElements(arrayObject, data, count); }
};

ref_t newShortArrayObject(const int16_t* data, size_t count);
int16_t* getShortArrayElements(ref_t arrayObject);
void releaseShortArrayElements(ref_t arrayObject, int16_t*, size_t count);

template<> class ArrayFunctions<int16_t> {
public:
    static ref_t newArrayObject(const int16_t* data, size_t count) { return newShortArrayObject(data, count); }
    static int16_t* getArrayObjectElements(ref_t arrayObject) { return getShortArrayElements(arrayObject); }
    static void releaseArrayObjectElements(ref_t arrayObject, int16_t* data, size_t count) { return releaseShortArrayElements(arrayObject, data, count); }
};

ref_t newByteArrayObject(const int8_t* data, size_t count);
int8_t* getByteArrayElements(ref_t arrayObject);
void releaseByteArrayElements(ref_t arrayObject, int8_t*, size_t count);

template<> class ArrayFunctions<int8_t> {
public:
    static ref_t newArrayObject(const int8_t* data, size_t count) { return newByteArrayObject(data, count); }
    static int8_t* getArrayObjectElements(ref_t arrayObject) { return getByteArrayElements(arrayObject); }
    static void releaseArrayObjectElements(ref_t arrayObject, int8_t* data, size_t count) { return releaseByteArrayElements(arrayObject, data, count); }
};

ref_t newFloatArrayObject(const float* data, size_t count);
float* getFloatArrayElements(ref_t arrayObject);
void releaseFloatArrayElements(ref_t arrayObject, float*, size_t count);

template<> class ArrayFunctions<float> {
public:
    static ref_t newArrayObject(const float* data, size_t count) { return newFloatArrayObject(data, count); }
    static float* getArrayObjectElements(ref_t arrayObject) { return getFloatArrayElements(arrayObject); }
    static void releaseArrayObjectElements(ref_t arrayObject, float* data, size_t count) { return releaseFloatArrayElements(arrayObject, data, count); }
};

ref_t newDoubleArrayObject(const double* data, size_t count);
double* getDoubleArrayElements(ref_t arrayObject);
void releaseDoubleArrayElements(ref_t arrayObject, double*, size_t count);

template<> class ArrayFunctions<double> {
public:
    static ref_t newArrayObject(const double* data, size_t count) { return newDoubleArrayObject(data, count); }
    static double* getArrayObjectElements(ref_t arrayObject) { return getDoubleArrayElements(arrayObject); }
    static void releaseArrayObjectElements(ref_t arrayObject, double* data, size_t count) { return releaseDoubleArrayElements(arrayObject, data, count); }
};

ref_t newStringArrayObject(const std::string* data, size_t count);
std::string* getStringArrayElements(ref_t arrayObject);
void releaseStringArrayElements(ref_t arrayObject, std::string*, size_t count);

template<> class ArrayFunctions<std::string> {
public:
    static ref_t newArrayObject(const std::string* data, size_t count) { return newStringArrayObject(data, count); }
    static std::string* getArrayObjectElements(ref_t arrayObject) { return getStringArrayElements(arrayObject); }
    static void releaseArrayObjectElements(ref_t arrayObject, std::string* data, size_t count) { return releaseStringArrayElements(arrayObject, data, count); }
};

ref_t newObjectArrayObject(const PassLocalRef<AnyObject>* data, size_t count);
std::vector<ref_t> getObjectArrayElementsData(ref_t arrayObject);
template<typename T> PassLocalRef<T>* getObjectArrayElements(std::vector<ref_t>&& elementsData);
void releaseObjectArrayElements(ref_t arrayObject, PassLocalRef<AnyObject>*, size_t count);

template<typename T> PassLocalRef<T>* getObjectArrayElements(std::vector<ref_t>&& elementsData)
{
    if (elementsData.size() < 1)
        return nullptr;

    PassLocalRef<T>* objects = reinterpret_cast<PassLocalRef<T>*>(malloc(sizeof(PassLocalRef<T>) * elementsData.size()));

    for (ref_t elementData : elementsData)
        new (objects + index) PassLocalRef<T>(T::fromRef(elementData));

    return objects;
}

template<typename T> class ArrayFunctions<PassLocalRef<T>> {
public:
    static ref_t newArrayObject(const PassLocalRef<T>* data, size_t count) { return newObjectArrayObject(reinterpret_cast<const PassLocalRef<AnyObject>*>(data), count); }
    static PassLocalRef<T>* getArrayObjectElements(ref_t arrayObject) { return getObjectArrayElements<T>(getObjectArrayElementsData(arrayObject)); }
    static void releaseArrayObjectElements(ref_t arrayObject, PassLocalRef<T>* data, size_t count) { return releaseObjectArrayElements(arrayObject, reinterpret_cast<PassLocalRef<AnyObject>*>(data), count); }
};

} // namespace JNI
