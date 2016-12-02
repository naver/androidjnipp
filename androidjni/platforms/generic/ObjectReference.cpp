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

#include "ObjectReference.h"

namespace JNI {

ref_t ObjectReference::refLocal()
{
    if (m_localRefCount == 0 && !m_referencedPtr)
        m_referencedPtr = m_dereferencedPtr.lock();

    ++m_localRefCount;
    return this;
}

void ObjectReference::derefLocal(ref_t ref)
{
    assert(ref == this);
    assert(m_localRefCount > 0);
    --m_localRefCount;
    deleteIfPossible();
}

ref_t ObjectReference::refGlobal()
{
    if (m_globalRefCount == 0 && !m_referencedPtr)
        m_referencedPtr = m_dereferencedPtr.lock();

    ++m_globalRefCount;
    return this;
}

void ObjectReference::derefGlobal(ref_t ref)
{
    assert(ref == this);
    assert(m_globalRefCount > 0);
    --m_globalRefCount;
    deleteIfPossible();
}

void ObjectReference::deleteIfPossible()
{
    if (m_localRefCount > 0 || m_globalRefCount > 0)
        return;

    if (m_referencedPtr) {
        m_dereferencedPtr = m_referencedPtr;
        m_referencedPtr.reset();
        return;
    }

    if (m_preventDeletion > 0)
        return;

    delete this;
}

} // namespace Managed
