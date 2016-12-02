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

import labs.naver.androidjni.CalledByNative;
import android.app.Activity;
import android.util.Log;
import android.view.MotionEvent;
import android.widget.TextView;
import android.os.Bundle;


public class TestActivity extends Activity
{
    private static final long INTERVAL_IN_MS = 3000;

    private StringGenerator mGenerator = new StringGenerator();
    private boolean mRequestedExit = false;

    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);

        /* Create a TextView and set its content.
         * the text is retrieved by calling a native
         * function.
         */
        final TextView  tv = new TextView(this);
        StringGeneratorClient client = new StringGeneratorClient() {
            @CalledByNative
            void stringFromJNI(String s) {
                super.stringFromJNI(s);
                tv.setText(s);
            }
        };
        mGenerator.setClient("TestActivityClient", client);
        tv.setText( mGenerator.stringFromJNI() );

        mGenerator.setWhat(new StringGeneratorClient() {
            protected void finalize() {
                Log.d("WeakObject", "Weak reference does not help me to live at all!");
            }

            @CalledByNative
            void stringFromJNI(String s) {
                Log.d("WeakObject", "Hey, I'm still alive! Here's the string: " + s);
            }
        });

        setContentView(tv);
        tv.postDelayed(new Runnable() {
            @Override
            public void run() {
                if (mRequestedExit)
                    return;

                mGenerator.generateNumberForJNI();
                mGenerator.requestStringFromJNI();
                tv.postDelayed(this, INTERVAL_IN_MS);
            }}, INTERVAL_IN_MS);
    }

    @Override
    public boolean onTouchEvent(MotionEvent event) {
        Log.d("TestActivity", "TestActivity will exit...");
        mRequestedExit = true;
        mGenerator = null;
        return super.onTouchEvent(event);
    }
}
