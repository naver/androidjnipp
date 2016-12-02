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

#include <android/java/lang/String.h>

namespace java {
namespace lang {
namespace Managed {

class StringPrivate : public String::Private {
public:
    std::string m_data;
};

static StringPrivate& string(String::Private& d)
{
    return static_cast<StringPrivate&>(d);
}

class String::NativeBindings {
public:
    static void setData(String& s, const char* chars)
    {
        string(*s.m_private).m_data = std::string(chars);
    }
};

void String::INIT()
{
    m_private = std::make_unique<StringPrivate>();
    string(*m_private).m_data = std::string("");
}

void String::INIT(const std::vector<int8_t>& data)
{
    m_private = std::make_unique<StringPrivate>();
    string(*m_private).m_data = std::string(reinterpret_cast<const char*>(data.data()), data.size());
}

std::shared_ptr<String> String::create(const char* chars)
{
    std::shared_ptr<String> string = String::create();
    String::NativeBindings::setData(*string, chars);
    return string;
}

std::vector<int8_t> String::getBytes()
{
    return std::vector<int8_t>(string(*m_private).m_data.data(), string(*m_private).m_data.data() + string(*m_private).m_data.length());
}

} // namespace Managed
} // namespace lang
} // namespace java
