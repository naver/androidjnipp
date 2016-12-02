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

#include <android/java/util/Vector.h>

namespace java {
namespace util {
namespace Managed {

class VectorPrivate : public Vector::Private {
public:
    std::vector<std::shared_ptr<void>> m_data;
};

static VectorPrivate& vector(Vector::Private& d)
{
    return static_cast<VectorPrivate&>(d);
}

class Vector::NativeBindings {
public:
    static std::vector<std::shared_ptr<void>>& data(Vector& v)
    {
        return vector(*v.m_private).m_data;
    }
};

void Vector::INIT()
{
    m_private = std::make_unique<VectorPrivate>();
}

// TODO: IMPLEMENT
void Vector::INIT(int32_t capacity)
{
    m_private = std::make_unique<VectorPrivate>();
}

// TODO: IMPLEMENT
void Vector::INIT(int32_t capacity
    , int32_t capacityIncrement)
{
    m_private = std::make_unique<VectorPrivate>();
}

void Vector::add(int32_t location
    , std::shared_ptr<void> object)
{
    std::vector<std::shared_ptr<void>>::iterator it = vector(*m_private).m_data.begin();
    vector(*m_private).m_data.insert(it + location, object);
}

bool Vector::add(std::shared_ptr<void> object)
{
    vector(*m_private).m_data.push_back(object);
    return true;
}

void Vector::addElement(std::shared_ptr<void> object)
{
    vector(*m_private).m_data.push_back(object);
}

int32_t Vector::capacity()
{
    return vector(*m_private).m_data.capacity();
}

void Vector::clear()
{
    vector(*m_private).m_data.clear();
}

// TODO: IMPLEMENT
std::shared_ptr<void> Vector::clone()
{
    return 0;
}

bool Vector::contains(std::shared_ptr<void> object)
{
    if (std::find(vector(*m_private).m_data.begin(), vector(*m_private).m_data.end(), static_cast<std::shared_ptr<void>>(object)) != vector(*m_private).m_data.end())
        return true;
    return false;
}

std::shared_ptr<void> Vector::elementAt(int32_t location)
{
    return vector(*m_private).m_data.at(location);
}

void Vector::ensureCapacity(int32_t minimumCapacity)
{
    vector(*m_private).m_data.reserve(minimumCapacity);
}

// TODO: IMPLEMENT
bool Vector::equals(std::shared_ptr<void> object)
{
    return 0;
}

std::shared_ptr<void> Vector::firstElement()
{
    return elementAt(0);
}

std::shared_ptr<void> Vector::get(int32_t location)
{
    return elementAt(location);
}

// TODO: IMPLEMENT
int32_t Vector::hashCode()
{
    return -1;
}

int32_t Vector::indexOf(std::shared_ptr<void> object)
{
    std::vector<std::shared_ptr<void>>::iterator it = 
        std::find(vector(*m_private).m_data.begin(), vector(*m_private).m_data.end(), static_cast<std::shared_ptr<void>>(object));

    if (it != vector(*m_private).m_data.end())
        return it - vector(*m_private).m_data.begin();
    
    return -1;        
}

int32_t Vector::indexOf(std::shared_ptr<void> object
    , int32_t location)
{
    std::vector<std::shared_ptr<void>>::iterator it = 
        std::find(vector(*m_private).m_data.begin() + location, vector(*m_private).m_data.end(), static_cast<std::shared_ptr<void>>(object));

    if (it != vector(*m_private).m_data.end())
        return it - vector(*m_private).m_data.begin();

    return -1;
}

void Vector::insertElementAt(std::shared_ptr<void> object
    , int32_t location)
{
    add(location, object);
}

bool Vector::isEmpty()
{
    return vector(*m_private).m_data.empty();
}

// TODO: IMPLEMENT
std::shared_ptr<void> Vector::lastElement()
{
    return 0;
}

// TODO: IMPLEMENT
int32_t Vector::lastIndexOf(std::shared_ptr<void> object)
{
    return 0;
}

// TODO: IMPLEMENT
int32_t Vector::lastIndexOf(std::shared_ptr<void> object
    , int32_t location)
{
    return 0;
}

std::shared_ptr<void> Vector::remove(int32_t location)
{
    std::vector<std::shared_ptr<void>>::iterator it = vector(*m_private).m_data.begin();
    std::advance(it, location);
    std::shared_ptr<void> ret = vector(*m_private).m_data.at(it - vector(*m_private).m_data.begin());
    vector(*m_private).m_data.erase(it);
    return ret;
}

bool Vector::remove(std::shared_ptr<void> object)
{
    if (vector(*m_private).m_data.erase(std::find(vector(*m_private).m_data.begin(), vector(*m_private).m_data.end(), static_cast<std::shared_ptr<void>>(object))) != vector(*m_private).m_data.end())
        return true;
    return false;
}

void Vector::removeAllElements()
{
    clear();
}

bool Vector::removeElement(std::shared_ptr<void> object)
{
    return remove(object);
}

void Vector::removeElementAt(int32_t location)
{
    remove(location);
}

// TODO: IMPLEMENT
std::shared_ptr<void> Vector::set(int32_t location
    , std::shared_ptr<void> object)
{
    return 0;
}

// TODO: IMPLEMENT
void Vector::setElementAt(std::shared_ptr<void> object
    , int32_t location)
{
}

void Vector::setSize(int32_t length)
{
    vector(*m_private).m_data.resize(length);
}

int32_t Vector::size()
{
    return vector(*m_private).m_data.size();
}

const Vector::Data& Vector::data()
{
    return Vector::NativeBindings::data(*this);
}

void Vector::trimToSize()
{
    vector(*m_private).m_data.resize(size());
}

} // namespace Natives
} // namespace util
} // namespace java
