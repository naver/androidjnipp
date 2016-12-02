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

#include <android/java/util/HashMap.h>

namespace java {
namespace util {
namespace Managed {

class HashMapPrivate : public HashMap::Private {
public:
    std::map<std::shared_ptr<void>, std::shared_ptr<void>> m_map;
};

static HashMapPrivate& map(HashMap::Private& d)
{
    return static_cast<HashMapPrivate&>(d);
}

class HashMap::NativeBindings {
public:
    static std::map<std::shared_ptr<void>, std::shared_ptr<void>>& data(HashMap& m)
    {
        return map(*m.m_private).m_map;
    }
};

void HashMap::INIT()
{
    m_private = std::make_unique<HashMapPrivate>();
}

void HashMap::INIT(int32_t capacity)
{
    m_private = std::make_unique<HashMapPrivate>();
}

void HashMap::INIT(int32_t capacity, float loadFactor)
{
    m_private = std::make_unique<HashMapPrivate>();
}


void HashMap::clear()
{
    map(*m_private).m_map.clear();
}

std::shared_ptr<void> HashMap::clone()
{
    return nullptr;
}

bool HashMap::containsKey(std::shared_ptr<void> key)
{
    return false;
}

bool HashMap::containsValue(std::shared_ptr<void> value)
{
    return false;
}

std::shared_ptr<void> HashMap::get(std::shared_ptr<void> key)
{
    return map(*m_private).m_map[key];
}

bool HashMap::isEmpty()
{
    return map(*m_private).m_map.empty();
}

std::shared_ptr<void> HashMap::put(std::shared_ptr<void> key, std::shared_ptr<void> value)
{
    map(*m_private).m_map.insert(std::pair<std::shared_ptr<void>, std::shared_ptr<void>>(key, value));
    return nullptr;
}

std::shared_ptr<void> HashMap::remove(std::shared_ptr<void> key)
{
    map(*m_private).m_map.erase(key);
    return nullptr;
}

int32_t HashMap::size()
{
    return map(*m_private).m_map.size();
}

const HashMap::Data& HashMap::data()
{
    return java::util::Managed::HashMap::NativeBindings::data(*this);
}

} // namespace Managed
} // namespace util
} // namespace java
