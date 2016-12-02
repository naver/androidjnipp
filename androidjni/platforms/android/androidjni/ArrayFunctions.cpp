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

#include "androidjni/PassArray.h"

#include "JavaVM.h"

namespace JNI {

void deleteArrayObject(ref_t arrayObject)
{
    if (arrayObject)
        getEnv()->DeleteLocalRef(reinterpret_cast<jobject>(arrayObject));
}

size_t getArrayObjectElementsCount(ref_t arrayObject)
{
    if (arrayObject)
        return getEnv()->GetArrayLength(reinterpret_cast<jarray>(arrayObject));
}

ref_t newIntArrayObject(const int32_t* data, size_t count)
{
    jintArray arrayObject = getEnv()->NewIntArray(count);
    if (!arrayObject)
        return 0;

    jint* arrayElements = getEnv()->GetIntArrayElements(arrayObject, NULL);
    if (arrayElements) {
        memcpy(arrayElements, data, sizeof(jint) * count);
        getEnv()->ReleaseIntArrayElements(arrayObject, arrayElements, 0);
    }

    return arrayObject;
}

int32_t* getIntArrayElements(ref_t arrayObject)
{
    return getEnv()->GetIntArrayElements(reinterpret_cast<jintArray>(arrayObject), NULL);
}

void releaseIntArrayElements(ref_t arrayObject, int32_t* data, size_t)
{
    if (!data)
        return;

    getEnv()->ReleaseIntArrayElements(reinterpret_cast<jintArray>(arrayObject), data, 0);
}

ref_t newShortArrayObject(const int16_t* data, size_t count)
{
    jshortArray arrayObject = getEnv()->NewShortArray(count);
    if (!arrayObject)
        return 0;

    jshort* arrayElements = getEnv()->GetShortArrayElements(arrayObject, NULL);
    if (arrayElements) {
        memcpy(arrayElements, data, sizeof(jshort) * count);
        getEnv()->ReleaseShortArrayElements(arrayObject, arrayElements, 0);
    }

    return arrayObject;
}

int16_t* getShortArrayElements(ref_t arrayObject)
{
    return getEnv()->GetShortArrayElements(reinterpret_cast<jshortArray>(arrayObject), NULL);
}

void releaseShortArrayElements(ref_t arrayObject, int16_t* data, size_t)
{
    if (!data)
        return;

    getEnv()->ReleaseShortArrayElements(reinterpret_cast<jshortArray>(arrayObject), data, 0);
}

ref_t newByteArrayObject(const int8_t* data, size_t count)
{
    jbyteArray arrayObject = getEnv()->NewByteArray(count);
    if (!arrayObject)
        return 0;

    jbyte* arrayElements = getEnv()->GetByteArrayElements(arrayObject, NULL);
    if (arrayElements) {
        memcpy(arrayElements, data, sizeof(jbyte) * count);
        getEnv()->ReleaseByteArrayElements(arrayObject, arrayElements, 0);
    }

    return arrayObject;
}

int8_t* getByteArrayElements(ref_t arrayObject)
{
    return getEnv()->GetByteArrayElements(reinterpret_cast<jbyteArray>(arrayObject), NULL);
}

void releaseByteArrayElements(ref_t arrayObject, int8_t* data, size_t)
{
    if (!data)
        return;

    getEnv()->ReleaseByteArrayElements(reinterpret_cast<jbyteArray>(arrayObject), data, 0);
}

ref_t newFloatArrayObject(const float* data, size_t count)
{
    jfloatArray arrayObject = getEnv()->NewFloatArray(count);
    if (!arrayObject)
        return 0;

    jfloat* arrayElements = getEnv()->GetFloatArrayElements(arrayObject, NULL);
    if (arrayElements) {
        memcpy(arrayElements, data, sizeof(jfloat) * count);
        getEnv()->ReleaseFloatArrayElements(arrayObject, arrayElements, 0);
    }

    return arrayObject;
}

float* getFloatArrayElements(ref_t arrayObject)
{
    return getEnv()->GetFloatArrayElements(reinterpret_cast<jfloatArray>(arrayObject), NULL);
}

void releaseFloatArrayElements(ref_t arrayObject, float* data, size_t)
{
    if (!data)
        return;

    getEnv()->ReleaseFloatArrayElements(reinterpret_cast<jfloatArray>(arrayObject), data, 0);
}

ref_t newDoubleArrayObject(const double* data, size_t count)
{
    jdoubleArray arrayObject = getEnv()->NewDoubleArray(count);
    if (!arrayObject)
        return 0;

    jdouble* arrayElements = getEnv()->GetDoubleArrayElements(arrayObject, NULL);
    if (arrayElements) {
        memcpy(arrayElements, data, sizeof(jdouble) * count);
        getEnv()->ReleaseDoubleArrayElements(arrayObject, arrayElements, 0);
    }

    return arrayObject;
}

double* getDoubleArrayElements(ref_t arrayObject)
{
    return getEnv()->GetDoubleArrayElements(reinterpret_cast<jdoubleArray>(arrayObject), NULL);
}

void releaseDoubleArrayElements(ref_t arrayObject, double* data, size_t)
{
    if (!data)
        return;

    getEnv()->ReleaseDoubleArrayElements(reinterpret_cast<jdoubleArray>(arrayObject), data, 0);
}

static jclass ClassID_java_lang_String()
{
    static jclass cid = reinterpret_cast<jclass>(getEnv()->NewGlobalRef(getEnv()->FindClass("java/lang/String")));
    return cid;
}

ref_t newStringArrayObject(const std::string* data, size_t count)
{
    jobjectArray arrayObject = getEnv()->NewStringArray(count, ClassID_java_lang_String(), NULL);
    if (!arrayObject)
        return 0;

    for (size_t index = 0; index < count; ++index)
        getEnv()->SetStringArrayElement(arrayObject, index, getEnv()->NewStringUTF(data[index].data()));

    return arrayObject;
}

std::string* getStringArrayElements(ref_t arrayObject)
{
    size_t count = getArrayObjectElementsCount(arrayObject);
    if (count < 1)
        return nullptr;

    std::string* strings = new std::string[count];

    for (size_t index = 0; index < count; ++index) {
        jstring element = (jstring)getEnv()->GetStringArrayElement(reinterpret_cast<jobjectArray>(arrayObject), index);

        const char* chars = getEnv()->GetStringUTFChars(element, NULL);
        if (chars) {
            jsize length = getEnv()->GetStringUTFLength(element);
            strings[index].assign(chars, length);
            getEnv()->ReleaseStringUTFChars(element, chars);
        }
    }

    return strings;
}

void releaseStringArrayElements(ref_t arrayObject, std::string* data, size_t)
{
    if (!data)
        return;

    delete [] data;
}

static jclass ClassID_java_lang_Object()
{
    static jclass cid = reinterpret_cast<jclass>(getEnv()->NewGlobalRef(getEnv()->FindClass("java/lang/Object")));
    return cid;
}

ref_t newObjectArrayObject(const PassLocalRef<AnyObject>* data, size_t count)
{
    jobjectArray arrayObject = getEnv()->NewObjectArray(count, ClassID_java_lang_Object(), NULL);
    if (!arrayObject)
        return 0;

    for (size_t index = 0; index < count; ++index)
        getEnv()->SetObjectArrayElement(arrayObject, index, reinterpret_cast<jobject>(data[index].get()));

    return arrayObject;
}

std::vector<ref_t> getObjectArrayElementsData(ref_t arrayObject)
{
    size_t count = getArrayObjectElementsCount(arrayObject);
    if (count < 1)
        return std::vector<ref_t>();

    std::vector<ref_t> objects(count, nullptr);

    for (size_t index = 0; index < count; ++index)
        objects[index] = getEnv()->GetObjectArrayElement(reinterpret_cast<jobjectArray>(arrayObject), index);

    return objects;
}

void releaseObjectArrayElements(ref_t arrayObject, PassLocalRef<AnyObject>* data, size_t count)
{
    if (!data)
        return;

    for (size_t index = 0; index < count; ++index)
        data[index].~PassLocalRef<AnyObject>();

    free(data);
}

}
