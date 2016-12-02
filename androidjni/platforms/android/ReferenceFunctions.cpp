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

#include "androidjni/ReferenceFunctions.h"

#include "JavaVM.h"

namespace JNI {

ref_t refLocal(ref_t ref)
{
    if (!ref)
        return 0;

    return CALL_JNI(NewLocalRef, ref);
}

void derefLocal(ref_t ref)
{
    if (!ref)
        return;

    CALL_JNI(DeleteLocalRef, ref);
}

ref_t refGlobal(ref_t ref)
{
    if (!ref)
        return 0;

    return CALL_JNI(NewGlobalRef, ref);
}

void derefGlobal(ref_t ref)
{
    if (!ref)
        return;

    CALL_JNI(DeleteGlobalRef, ref);
}

bool isExpiredWeakGlobal(weak_t ref)
{
    if (CALL_JNI(IsSameObject, ref, NULL))
        return true;

    return false;
}

weak_t refWeakGlobal(ref_t ref)
{
    if (!ref)
        return 0;

    jweak weak = CALL_JNI(NewWeakGlobalRef, ref);
    return reinterpret_cast<weak_t>(weak);
}

void derefWeakGlobal(weak_t ref)
{
    if (!ref)
        return;

    CALL_JNI(DeleteWeakGlobalRef, ref);
}

struct LocalCallerObject {
    ref_t oref;
    LocalCallerObject* next;
};

static thread_local LocalCallerObject localCallerObjects;

ref_t popLocalCallerObjectRef()
{
    ref_t ref = 0;
    std::swap(ref, localCallerObjects.oref);

    if (!localCallerObjects.next)
        return ref;

    std::unique_ptr<LocalCallerObject> next(localCallerObjects.next);
    localCallerObjects.oref = next->oref;
    localCallerObjects.next = next->next;
    return ref;
}

void pushLocalCallerObjectRef(ref_t ref)
{
    if (!localCallerObjects.oref) {
        localCallerObjects.oref = ref;
        return;
    }

    LocalCallerObject* next = new LocalCallerObject;
    next->oref = localCallerObjects.oref;
    next->next = localCallerObjects.next;
    localCallerObjects.oref = ref;
    localCallerObjects.next = next;
}

} // namespace JNI
