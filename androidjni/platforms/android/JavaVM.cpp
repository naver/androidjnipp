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

#include "JavaVM.h"

#include <dlfcn.h>
#include <mutex>
#include <string>

namespace JNI {

class ThreadDestructor {
public:
    static ThreadDestructor* get();

    void setAttached() { m_attached = true; }

private:
    ThreadDestructor()
        : m_attached(false)
    { }
    ~ThreadDestructor()
    {
        if (!m_attached)
            return;

        // We need to explicitly call DetachCurrentThread() before exiting the thread.
        // Otherwise, dalvikvm bug checks and aborts: "thread exiting, not yet detached".
        // It is harmless to call DetachCurrentThread() when we have not called AttachCurrentThread().
        JavaVM* jvm = getVM();
        if (jvm)
            jvm->DetachCurrentThread();
    }

    static void destroy(void*);

    bool m_attached;
};

ThreadDestructor* ThreadDestructor::get()
{
    static std::once_flag onceFlag;
    static pthread_key_t threadDestructorKey;
    std::call_once(onceFlag, []{
        pthread_key_create(&threadDestructorKey, destroy);
    });

    void* data = pthread_getspecific(threadDestructorKey);
    if (!data) {
        data = new ThreadDestructor;
        pthread_setspecific(threadDestructorKey, data);
    }

    return static_cast<ThreadDestructor*>(data);
}

void ThreadDestructor::destroy(void* data)
{
    ThreadDestructor* destructor = static_cast<ThreadDestructor*>(data);
    delete destructor;
}

JNIEnv* getEnv()
{
    union {
        JNIEnv* env;
        void* dummy;
    } u;
    jint jniError = 0;

    jniError = getVM()->AttachCurrentThread(&u.env, 0);
    if (jniError == JNI_OK) {
        ThreadDestructor::get()->setAttached();
        return u.env;
    }

    ALOGE("AttachCurrentThread failed, returned %ld", static_cast<long>(jniError));
    return 0;
}

#if !defined(WIN32)
// Code from Webkit (https://webkit.org/) under LGPL v2 and BSD licenses (https://webkit.org/licensing-webkit/)
static jint KJSGetCreatedJavaVMs(JavaVM** vmBuf, jsize bufLen, jsize* nVMs)
{
    static void* javaVMFramework = 0;
    if (!javaVMFramework)
        javaVMFramework = dlopen("/System/Library/Frameworks/JavaVM.framework/JavaVM", RTLD_LAZY);
    if (!javaVMFramework)
        return JNI_ERR;

    typedef jint(*FunctionPointerType)(JavaVM**, jsize, jsize*);
    static FunctionPointerType functionPointer = 0;
    if (!functionPointer)
        functionPointer = reinterpret_cast<FunctionPointerType>(dlsym(javaVMFramework, "JNI_GetCreatedJavaVMs"));
    if (!functionPointer)
        return JNI_ERR;
    return functionPointer(vmBuf, bufLen, nVMs);
}
#endif

static JavaVM* jvm = 0;

// Provide the ability for an outside component to specify the JavaVM to use
// If the jvm value is set, the getJavaVM function below will just return.
// In getJNIEnv(), if AttachCurrentThread is called to a VM that is already
// attached, the result is a no-op.
void setVM(JavaVM* javaVM)
{
    jvm = javaVM;
}

JavaVM* getVM()
{
    if (jvm)
        return jvm;

#if !defined(WIN32)
    JavaVM* jvmArray[1];
    jsize bufLen = 1;
    jsize nJVMs = 0;
    jint jniError = 0;

    // Assumes JVM is already running ..., one per process
    jniError = KJSGetCreatedJavaVMs(jvmArray, bufLen, &nJVMs);
    if (jniError == JNI_OK && nJVMs > 0)
        jvm = jvmArray[0];
    else
        ALOGE("JNI_GetCreatedJavaVMs failed, returned %ld", static_cast<long>(jniError));
#endif
    return jvm;
}

jstring toManaged(const std::string& value)
{
    return getEnv()->NewStringUTF(value.data());
}

std::string toNative(jstring str)
{
    if (!str)
        return std::string();

    const char* chars = getEnv()->GetStringUTFChars(str, NULL);
    if (!chars)
        return std::string();

    std::string nativeString(chars);
    getEnv()->ReleaseStringUTFChars(str, chars);
    return nativeString;
}

}
