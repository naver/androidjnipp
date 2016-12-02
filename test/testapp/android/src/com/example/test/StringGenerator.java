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

package com.example.test;

import java.util.Random;

import android.util.Log;

import labs.naver.androidjni.AbstractMethod;
import labs.naver.androidjni.AccessedByNative;
import labs.naver.androidjni.CalledByNative;
import labs.naver.androidjni.NativeExportMacro;
import labs.naver.androidjni.NativeConstructor;
import labs.naver.androidjni.NativeDestructor;
import labs.naver.androidjni.NativeNamespace;
import labs.naver.androidjni.NativeObjectField;

@NativeNamespace("com.example.test")
@NativeExportMacro("TESTLIB_EXPORT")
public class StringGenerator {

    @AccessedByNative
    private static final int IMMUTABLE = 2014;
    @AccessedByNative
    private static final int IMMUTABLE2 = IMMUTABLE;
    @AccessedByNative
    private static int mMutableStatic = -1;
    @AccessedByNative
    private int mMutable = 0;
    @AccessedByNative
    private int mAnotherMutable = 0;

    @AccessedByNative
    private static final String IMMUTABLE_STRING = "an immutable string";
    @AccessedByNative
    private static String mMutableStaticString = "a mutable string";
    @AccessedByNative
    private String mMutableString = "yet another mutable string";

    @CalledByNative
    public StringGenerator() {
        // Constructor called from Java or Native.
        // Initiate native construction here, if needed.
        nativeCreate();

        mMutableStatic = 2015;
    }

    @NativeObjectField
    private int mNativePtr;

    protected void finalize() {
        Log.d("NativeObject", "Called finalize()");
        nativeDestroy();
    }

    private Random mRandom = new Random();

    public void generateNumberForJNI() {
        mMutable = mRandom.nextInt((9999 - 1000) + 1) + 1000;
    }

    @CalledByNative
    private int getNumber() {
        return 1627;
    }

    /* A native method that is implemented by the
     * 'testlib' native library, which is packaged
     * with this application.
     */
    public native String  stringFromJNI();

    /* This is another native method declaration that is *not*
     * implemented by 'testlib'. This is simply to show that
     * you can declare as many native methods in your Java code
     * as you want, their implementation is searched in the
     * currently loaded native libraries only the first time
     * you call them.
     *
     * Trying to call this function will result in a
     * java.lang.UnsatisfiedLinkError exception !
     */
    public native String  unimplementedStringFromJNI();

    // Sets listener client to StringGenerator.
    public native void    setClient(String name, StringGeneratorClient client);

    @CalledByNative
    public StringGeneratorClient getClient() {
        return (StringGeneratorClient)nativeGetClient();
    }
    public native Object  nativeGetClient();

    public native void    requestStringFromJNI();

    @CalledByNative
    public void setWhat(Object what) {
        nativeSetWhat(what);
    }
    private native void   nativeSetWhat(Object what);

    // Link to native constructor/destructors.
    @NativeConstructor
    private native void   nativeCreate();
    @NativeConstructor
    private native void   nativeCreateWithParameter(int i);
    @NativeDestructor
    private native void   nativeDestroy();

    /* this is used to load the 'testlib' library on application
     * startup. The library has already been unpacked into
     * /data/data/com.example.test/lib/libtestlib.so at
     * installation time by the package manager.
     */
    static {
        System.loadLibrary("testlib");
    }
}
