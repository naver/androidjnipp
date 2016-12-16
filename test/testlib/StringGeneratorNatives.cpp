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

#include <com/example/test/Natives/StringGenerator.h>

#include <JNI/java/lang/Integer.h>
#include <JNI/java/lang/String.h>
#include <JNI/java/util/Vector.h>
#include <com/example/test/Natives/StringGeneratorClient.h>

#include <androidjni/GlobalRef.h>
#include <androidjni/MarshalingHelpers.h>
#include <androidjni/WeakGlobalRef.h>

#include <sstream>

#ifdef ANDROID
#include <androidjni/JavaVM.h>
#endif

namespace com {
namespace example {
namespace test {
namespace Natives {

template <typename T>
std::string to_string(T value)
{
    std::ostringstream os ;
    os << value ;
    return os.str() ;
}

class StringGeneratorPrivate : public StringGenerator::Private {
public:
    JNI::GlobalRef<StringGeneratorClient> client;
    JNI::WeakGlobalRef<JNI::AnyObject> what;

    ~StringGeneratorPrivate()
    {
#ifdef ANDROID
        ALOGD("StringGenerator::Private is being destroyed...");
#endif
    }
};

static StringGeneratorPrivate* ptr(const std::unique_ptr<StringGenerator::Private>& p)
{
    return static_cast<StringGeneratorPrivate*>(p.get());
}

// TODO: IMPLEMENT
StringGenerator* StringGenerator::nativeCreate()
{
    StringGenerator* nativeObject = new StringGenerator;
    nativeObject->m_private.reset(new StringGeneratorPrivate);
    return nativeObject;
}

// TODO: IMPLEMENT
StringGenerator* StringGenerator::nativeCreateWithParameter(int32_t i)
{
    StringGenerator* nativeObject = new StringGenerator;
    nativeObject->m_private.reset(new StringGeneratorPrivate);
    return nativeObject;
}

// TODO: IMPLEMENT
std::string StringGenerator::stringFromJNI()
{
    int32_t numberFromGetNumber = getNumber();
    int32_t numberFromStatic = mMutableStatic.get();
    int32_t numberFromInstance = mMutable.get();
    int32_t numberToAnother = numberFromGetNumber + numberFromStatic + numberFromInstance;

    mAnotherMutable.set(numberToAnother);

    return std::string("Hello from JNI! #") + to_string(numberFromGetNumber)
            + std::string(" + ") + to_string(numberFromStatic)
            + std::string(" + ") + to_string(numberFromInstance)
            + std::string(" = ") + to_string(numberToAnother);
}

// TODO: IMPLEMENT
std::string StringGenerator::unimplementedStringFromJNI()
{
    return std::string();
}

// TODO: IMPLEMENT
JNI::PassLocalRef<JNI::AnyObject> StringGenerator::nativeGetClient()
{
    return ptr(m_private)->client;
}

// TODO: IMPLEMENT
void StringGenerator::setClient(const std::string& name, JNI::PassLocalRef<StringGeneratorClient> client)
{
#ifdef ANDROID
    ALOGD("StringGenerator::setClient name=%s, client=%p:%p", name.data(), client.get(), client.getPtr());
#endif
    ptr(m_private)->client = client;
}

// TODO: IMPLEMENT
void StringGenerator::nativeSetWhat(JNI::PassLocalRef<JNI::AnyObject> what)
{
#ifdef ANDROID
    ALOGD("StringGenerator::nativeSetWhat what=%p:%p", what.get(), what.getPtr());
#endif
    ptr(m_private)->what = what;
}

// TODO: IMPLEMENT
void StringGenerator::requestStringFromJNI()
{
    std::string resultString = stringFromJNI();
    ptr(m_private)->client->stringFromJNI(resultString);

    JNI::LocalRef<Vector> v = Vector::create(10);
    v->add(java::lang::String::create(resultString));
    ptr(m_private)->client->stringFromJNI(v);
    v.reset();

    JNI::LocalRef<StringGeneratorClient> weakClient = ptr(m_private)->what.tryPromote<StringGeneratorClient>();
    if (!weakClient) {
#ifdef ANDROID
        ALOGD("Failed to promote weak client...");
#endif
    } else {
        weakClient->stringFromJNI("Called from weak client...");
    }
}

} // namespace Natives
} // namespace test
} // namespace example
} // namespace com
