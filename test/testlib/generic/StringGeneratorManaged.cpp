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

#include <com/example/test/StringGeneratorManagedBase.h>

#include <com/example/test/StringGeneratorClientManagedBase.h>

namespace com {
namespace example {
namespace test {
namespace Managed {

// TODO: IMPLEMENT
void StringGenerator::INIT()
{
    nativeCreate();
}

// TODO: IMPLEMENT
int32_t StringGenerator::getNumber()
{
    return 1315;
}

// TODO: IMPLEMENT
std::shared_ptr<StringGeneratorClient> StringGenerator::getClient()
{
    return std::static_pointer_cast<StringGeneratorClient>(nativeGetClient());
}

// TODO: IMPLEMENT
void StringGenerator::setWhat(std::shared_ptr<void> what)
{
    nativeSetWhat(what);
}

} // namespace Managed
} // namespace test
} // namespace example
} // namespace com
