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

#include <android/android/graphics/Point.h>

namespace android {
namespace graphics {
namespace Managed {

void Point::INIT()
{
    this->x = 0;
    this->y = 0;
}

void Point::INIT(int32_t x
    , int32_t y)
{
    this->x = x;
    this->y = y;
}

void Point::INIT(std::shared_ptr<Managed::Point> src)
{
    x = src->x;
    y = src->y;
}

void Point::set(int32_t x
    , int32_t y)
{
    this->x = x;
    this->y = y;
}

void Point::negate()
{
    x = -x;
    y = -y;
}

void Point::offset(int32_t dx
    , int32_t dy)
{
    x += dx;
    y += dy;
}

bool Point::equals(int32_t x
    , int32_t y)
{
    return this->x == x && this->y == y;
}

bool Point::equals(std::shared_ptr<void> o)
{
    if (this == o.get())
        return true;
    if (o.get() == nullptr)
        return false;

    Point* point = (Point*)o.get();

    if (x != point->x)
        return false;
    if (y != point->y)
        return false;

    return true;
}

int32_t Point::hashCode()
{
    int result = x;
    result = 31 * result + y;
    return result;
}

} // namespace Managed
} // namespace graphics
} // namespace android
