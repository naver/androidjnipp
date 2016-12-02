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

#pragma once

#include "ArrayFunctions.h"

#include <array>

namespace JNI {

template<typename T>
class JNI_EXPORT PassArray final {
public:
    PassArray(const T* data, size_t count, bool copyData = false)
        : m_data(data)
        , m_count(count)
        , m_ref(ArrayFunctions<T>::newArrayObject(m_data, m_count))
        , m_elements(0)
    {
    }
    PassArray(const PassArray& array)
        : m_data(0)
        , m_count(getArrayObjectElementsCount(array.m_ref))
        , m_ref(JNI::refLocal(array.m_ref))
        , m_elements(0)
    {
    }
    PassArray(ref_t array)
        : m_data(0)
        , m_count(getArrayObjectElementsCount(array))
        , m_ref(JNI::refLocal(array))
        , m_elements(0)
    {
    }
    template<size_t N> PassArray(const std::array<T, N>& array)
        : m_data(array.data())
        , m_count(array.size())
        , m_ref(ArrayFunctions<T>::newArrayObject(m_data, m_count))
        , m_elements(0)
    {
    }
    PassArray(const std::string& string)
        : m_data(reinterpret_cast<const int8_t*>(string.data()))
        , m_count(string.size())
        , m_ref(ArrayFunctions<T>::newArrayObject(m_data, m_count))
        , m_elements(0)
    {
    }
    PassArray(const std::vector<T>& vector)
        : m_data(vector.data())
        , m_count(vector.size())
        , m_ref(ArrayFunctions<T>::newArrayObject(m_data, m_count))
        , m_elements(0)
    {
    }
    ~PassArray()
    {
        ArrayFunctions<T>::releaseArrayObjectElements(m_ref, m_elements, m_count);
        deleteArrayObject(m_ref);
    }

    const T* data() const { return (m_data) ? m_data : (m_data = m_elements = ArrayFunctions<T>::getArrayObjectElements(m_ref)); }
    size_t count() const { return m_count; }

    ref_t leak() const
    {
        ref_t oref = 0;
        std::swap(oref, m_ref);
        return oref;
    }

private:
    mutable const T* m_data;
    size_t m_count;
    mutable ref_t m_ref;
    mutable T* m_elements;
}; // class PassArray

} // namespace JNI
