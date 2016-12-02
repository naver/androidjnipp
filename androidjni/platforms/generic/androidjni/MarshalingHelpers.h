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

#include "PassArray.h"
#include "ObjectReference.h"
#include <androidjni/PassLocalRef.h>

namespace JNI {

template<typename T>
T&& toManaged(T&& value)
{
    return std::forward<T>(value);
}

template<typename T>
T&& toNative(T&& value)
{
    return std::forward<T>(value);
}

template<typename T, typename U>
std::shared_ptr<T> toManaged(const PassLocalRef<U>& ref)
{
    return std::static_pointer_cast<T>(sharePtr(ref.get()));
}

template<typename T, typename U>
PassLocalRef<T> toNative(std::shared_ptr<U>& ref)
{
    return (ref) ? T::fromPtr(ref) : nullptr;
}

template<> inline std::shared_ptr<void> toManaged(const PassLocalRef<AnyObject>& ref)
{
    return sharePtr(ref.get());
}

template<> inline PassLocalRef<AnyObject> toNative(std::shared_ptr<void>& ref)
{
    return JNI::adoptRef(new ObjectReference(ref), reinterpret_cast<AnyObject*>(nullptr));
}

template<typename T>
inline std::vector<T> toManaged(PassArray<T> value)
{
    return std::vector<T>(value.data(), value.data() + value.count());
}

template<typename T>
inline PassArray<T> toNative(std::vector<T>& value)
{
    return PassArray<T>(value.data(), value.size(), true);
}

template<typename T, typename U>
inline std::vector<std::shared_ptr<T>> toManaged(PassArray<PassLocalRef<U>> value)
{
    return value.vectorize<T>();
}

}
