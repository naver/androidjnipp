set(ANDROIDJNI_HEADERS
    Abbreviations.h
    AnyObject.h
    GlobalRef.h
    JNIExportMacros.h
    JNIIncludes.h
    LocalRef.h
    NativeObject.h
    PassLocalRef.h
    ReferenceFunctions.h
    WeakGlobalRef.h
)

if (ANDROID)
    list(APPEND ANDROIDJNI_HEADERS
        platforms/android/AndroidJNI.h
        platforms/android/AndroidLog.h
        platforms/android/JavaVM.h

        platforms/android/androidjni/ArrayFunctions.h
        platforms/android/androidjni/MarshalingHelpers.h
        platforms/android/androidjni/PassArray.h
    )

    list(APPEND ANDROIDJNI_SOURCES
        platforms/android/JavaVM.cpp
        platforms/android/ReferenceFunctions.cpp

        platforms/android/androidjni/ArrayFunctions.cpp
    )
else ()
    list(APPEND ANDROIDJNI_HEADERS
        platforms/generic/ObjectReference.h

        platforms/generic/androidjni/MarshalingHelpers.h
        platforms/generic/androidjni/PassArray.h
    )

    list(APPEND ANDROIDJNI_SOURCES
        platforms/generic/ObjectReference.cpp
        platforms/generic/ReferenceFunctions.cpp
    )
endif ()

include_directories(
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/platforms/${TARGET_PLATFORM}"
    "${CMAKE_SOURCE_DIR}"
    "${CMAKE_CURRENT_BINARY_DIR}"
    "${CMAKE_BINARY_DIR}"
)

add_custom_command(
    OUTPUT  ${LIBRARY_PRODUCT_DIR}/scripts/interface-generator.py
    MAIN_DEPENDENCY ${GENERATOR_SCRIPT}
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/generator/scripts ${LIBRARY_PRODUCT_DIR}/scripts
    VERBATIM)

add_subdirectory(annotations)

add_definitions(-DBUILDING_JNILIB -DJNI_STATIC)

WRAP_SOURCELIST(${ANDROIDJNI_HEADERS} ${ANDROIDJNI_SOURCES})

add_library(androidjni++ ${ANDROIDJNI_HEADERS} ${ANDROIDJNI_SOURCES} ${GENERATOR_SCRIPT} $<TARGET_OBJECTS:android>)

ADD_PREFIX_HEADER(androidjni++ JNIExportMacros.h)
ADD_POST_BUILD_COMMAND(androidjni++)

COPY_LIBRARY_HEADERS(androidjni++ "${ANDROIDJNI_HEADERS}" include/androidjni++/androidjni)

set(ANDROID_SOURCE_DIR ${CMAKE_SOURCE_DIR}/android)
set(ANDROID_BINARY_DIR ${CMAKE_BINARY_DIR}/android)

COPY_LIBRARY_HEADERS_DIRECTORY(androidjni++ "${ANDROID_SOURCE_DIR}" include/androidjni++/android)
COPY_LIBRARY_HEADERS_DIRECTORY(androidjni++ "${ANDROID_BINARY_DIR}/GeneratedFiles" include/androidjni++/android)
