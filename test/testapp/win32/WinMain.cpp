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

#include <windows.h>
#include <tchar.h>

#include <test/testlib/TestLibPrefix.h>
#include <com/example/test/Managed/StringGenerator.h>
#include <com/example/test/Managed/StringGeneratorClient.h>

#include <android/graphics/Rect.h>
#include <java/lang/String.h>
#include <java/util/Vector.h>

using namespace com::example::test::Managed;

class HelloJniStringGeneratorClient : public StringGeneratorClient {
    void stringFromJNI(const std::string& s) override {
    }

    void stringFromJNI(std::shared_ptr<java_util_Managed_Vector> v) override {
    }

    void stringFromJNI(const std::vector<std::string>& i) override {
    }
};

int _tmain(int argc, _TCHAR* argv[])
{
    std::shared_ptr<StringGenerator> generator = StringGenerator::create();
    std::shared_ptr<StringGeneratorClient> generatorClient = StringGeneratorClient::create<HelloJniStringGeneratorClient>();
    StringGenerator::mMutableStatic = 2015;
    generator->mMutable = 1234;
    generator->setClient("TestActivityClient", generatorClient);
    generator->stringFromJNI();
    generator->requestStringFromJNI();
    std::shared_ptr<StringGeneratorClient> generatorClientRef = generator->getClient();
    std::shared_ptr<java::lang::String> string = java::lang::String::create("Hello, World!");
    std::shared_ptr<java::util::Vector> stringVector = java::util::Vector::create();
    std::shared_ptr<android::graphics::Rect> rect = android::graphics::Rect::create(0, 0, 10, 10);
    stringVector->add(string);
    generator->setWhat(stringVector);
    generator->setWhat(generatorClientRef);
    generator.reset();
    generatorClientRef.reset();
    generatorClient.reset();
    return 0;
}

