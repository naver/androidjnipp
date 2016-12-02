# androidjni++
While writing code in Java is fairly convienient, writing JNI code is not.
By using androidjni++ in your Android project you can remove effort for boring and painful JNI code writing.
Another alternative for get the things done is more famous SWIG, but androidjni++ should be better choice for this particular case for reasons below:
* You don't have to learn to write interface definition files for Java classes.
* You don't have to worry about your project being messed up by generated Java source files.
* You don't have to guess corresponding class methods by reading C functions.
* You don't have to care about Java reference management by using JNI functions.

And last but not least, androidjni++ supports generating wrappers in C++.
This enables you to write native modules for cross-platform usage much easier.

## Prerequisite
* Python - 2.7 or later needed.
* CMake - 3.4 or later.
* Android NDK - r12 or later preferred.

For cross-platform development on Windows, you'll need Visual Studio 2015.

## Getting Started
### Building Binaries for Android
With CMake and NDK installed, do like this:
```
export ANDROID_NDK=<path-to-ndk>
export PATH=$PATH:<path-to-android-sdk/tools>
mkdir build
cd build
cmake -DCMAKE_TOOLCHAIN_FILE=../android.toolchain.cmake -DCMAKE_BUILD_TYPE=Release -DANDROID_TOOLCHAIN_NAME=clang -DANDROID_ABI="armeabi-v7a with NEON" -DLIBRARY_PRODUCT_DIR=<output-directory> ..
cmake --build .
```

### Building Binaries for Windows
With CMake installed, do like this:
```
mkdir build
cd build
cmake -G "Visual Studio 14 2015" -D LIBRARY_PRODUCT_DIR=<output-directory> ..
```
Then open androidjni++.sln, Hit "Build Solution". or type `cmake --build .`

### Building Binaries for Other Platforms
Not yet supported. However we believe most of the implementation for Windows can be used as-is,
for platforms using C++ as their native language.

### Intergrating with Your Android Project
* Compile with annotations .java file and native .h header files.
* Link androidjni++.jar and libandroidjni++.so while building.
* You should directly use interface-generator.py for generating stub source files. Do it in generation step while running build script.
* Stubs for native interfaces are generated automatically, but you'll have to write the actual logic yourself.
Otherwise you'll suffer link errors. Link errors are useful for identifying what you should do.

## Resources(Under construction)
* The Idea Behind androidjni++
* Code Generation
* Java Reference Management
* Quick Tutorials

## Sample Code
See test/testapp.

## License
androidjni++ is licensed under BSD license.

```
Copyright (c) 2016 NAVER Corp.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, 
   this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, 
   this list of conditions and the following disclaimer in the documentation 
   and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its contributors 
   may be used to endorse or promote products derived from this software 
   without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE 
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
POSSIBILITY OF SUCH DAMAGE.
```