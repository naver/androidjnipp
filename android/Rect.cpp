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

#include <android/android/graphics/Rect.h>

#include <algorithm>

namespace android {
namespace graphics {
namespace Managed {

void Rect::INIT()
{
}

void Rect::INIT(int32_t left
    , int32_t top
    , int32_t right
    , int32_t bottom)
{
    this->left = left;
    this->top = top;
    this->right = right;
    this->bottom = bottom;
}

void Rect::INIT(std::shared_ptr<Managed::Rect> r)
{
    this->left = r->left;
    this->top = r->top;
    this->right = r->right;
    this->bottom = r->bottom;
}

bool Rect::equals(std::shared_ptr<void> o)
{
    if (this == o.get())
        return true;
    if (o.get() == nullptr)
        return false;

    Rect* r = (Rect*)o.get();
    return left == r->left && top == r->top
        && right == r->right && bottom == r->bottom;
}

int32_t Rect::hashCode()
{
    int32_t result = left;
    result = 31 * result + top;
    result = 31 * result + right;
    result = 31 * result + bottom;
    return result;
}

bool Rect::isEmpty()
{
    return left >= right || top >= bottom;
}

int32_t Rect::width()
{
    return right - left;
}

int32_t Rect::height()
{
    return bottom - top;
}

int32_t Rect::centerX()
{
    return (left + right) >> 1;
}

int32_t Rect::centerY()
{
    return (top + bottom) >> 1;
}

float Rect::exactCenterX()
{
    return (left + right) * 0.5f;
}

float Rect::exactCenterY()
{
    return (top + bottom) * 0.5f;
}

void Rect::setEmpty()
{
    left = right = top = bottom = 0;
}

void Rect::set(int32_t left
    , int32_t top
    , int32_t right
    , int32_t bottom)
{
    this->left = left;
    this->top = top;
    this->right = right;
    this->bottom = bottom;
}

void Rect::set(std::shared_ptr<Managed::Rect> src)
{
    this->left = src->left;
    this->top = src->top;
    this->right = src->right;
    this->bottom = src->bottom;
}

void Rect::offset(int32_t dx
    , int32_t dy)
{
    left += dx;
    top += dy;
    right += dx;
    bottom += dy;
}

void Rect::offsetTo(int32_t newLeft
    , int32_t newTop)
{
    right += newLeft - left;
    bottom += newTop - top;
    left = newLeft;
    top = newTop;
}

void Rect::inset(int32_t dx
    , int32_t dy)
{
    left += dx;
    top += dy;
    right -= dx;
    bottom -= dy;
}

bool Rect::contains(int32_t x
    , int32_t y)
{
    return left < right && top < bottom  // check for empty first
        && x >= left && x < right && y >= top && y < bottom;
}

bool Rect::contains(int32_t left
    , int32_t top
    , int32_t right
    , int32_t bottom)
{
    // check for empty first
    return this->left < this->right && this->top < this->bottom
        // now check for containment
        && this->left <= left && this->top <= top
        && this->right >= right && this->bottom >= bottom;
}

bool Rect::contains(std::shared_ptr<Managed::Rect> r)
{
    // check for empty first
    return this->left < this->right && this->top < this->bottom
        // now check for containment
        && left <= r->left && top <= r->top && right >= r->right && bottom >= r->bottom;
}

bool Rect::intersect(int32_t left
    , int32_t top
    , int32_t right
    , int32_t bottom)
{
    if (this->left < right && left < this->right && this->top < bottom && top < this->bottom) {
        if (this->left < left) this->left = left;
        if (this->top < top) this->top = top;
        if (this->right > right) this->right = right;
        if (this->bottom > bottom) this->bottom = bottom;
        return true;
    }
    return false;
}

bool Rect::intersect(std::shared_ptr<Managed::Rect> r)
{
    return intersect(r->left, r->top, r->right, r->bottom);
}

bool Rect::setIntersect(std::shared_ptr<Managed::Rect> a
    , std::shared_ptr<Managed::Rect> b)
{
    if (a->left < b->right && b->left < a->right && a->top < b->bottom && b->top < a->bottom) {
        left = std::max(a->left, b->left);
        top = std::max(a->top, b->top);
        right = std::min(a->right, b->right);
        bottom = std::min(a->bottom, b->bottom);
        return true;
    }
    return false;
}

bool Rect::intersects(int32_t left
    , int32_t top
    , int32_t right
    , int32_t bottom)
{
    return this->left < right && left < this->right && this->top < bottom && top < this->bottom;
}

bool Rect::intersects(std::shared_ptr<Managed::Rect> a
    , std::shared_ptr<Managed::Rect> b)
{
    return a->left < b->right && b->left < a->right && a->top < b->bottom && b->top < a->bottom;
}

void Rect::sort()
{
    if (left > right) {
        int32_t temp = left;
        left = right;
        right = temp;
    }
    if (top > bottom) {
        int32_t temp = top;
        top = bottom;
        bottom = temp;
    }
}

void Rect::scale(float scale)
{
    if (scale != 1.0f) {
        left = (int32_t)(left * scale + 0.5f);
        top = (int32_t)(top * scale + 0.5f);
        right = (int32_t)(right * scale + 0.5f);
        bottom = (int32_t)(bottom * scale + 0.5f);
    }
}

} // namespace Managed
} // namespace graphics
} // namespace android
