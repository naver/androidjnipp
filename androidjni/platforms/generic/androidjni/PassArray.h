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

#include <androidjni/AnyObject.h>

#include <array>
#include <vector>

namespace JNI {

template<typename T>
class JNI_EXPORT PassArray final {
public:
    PassArray(const T* data, size_t count, bool makeCopy = false)
        : m_data(data)
        , m_count(count)
        , m_copy(0)
    {
        if (makeCopy)
            copyData();
    }
    PassArray(const PassArray& array)
        : m_data(array.m_data)
        , m_count(array.m_count)
        , m_copy(0)
    {
        std::swap(m_copy, array.m_copy);
    }
    PassArray(const std::vector<T>& vector)
        : m_data(vector.data())
        , m_count(vector.size())
        , m_copy(0)
    {
        copyData();
    }
    ~PassArray()
    {
        deleteDataIfNeeded();
    }

    const T* data() const { return m_data; }
    size_t count() const { return m_count; }

    template<typename U>
    std::vector<std::shared_ptr<U>> vectorize()
    {
        std::vector<std::shared_ptr<U>> v;
        for (size_t i = 0; i < m_count; ++i)
            v.push_back(toManaged<U>(m_data[i]));
        return v;
    }

private:
    void copyData()
    {
        m_copy = new T[m_count];
        memcpy(m_copy, m_data, m_count * sizeof(T));
        m_data = m_copy;
    }
    void deleteDataIfNeeded()
    {
        if (m_copy)
            delete [] m_copy;
    }

    const T* m_data;
    size_t m_count;
    mutable T* m_copy;
}; // class PassArray

} // namespace JNI
