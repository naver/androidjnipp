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

set(ANDROIDJNI_INTERFACES
    interfaces/Boolean.in
    interfaces/HashMap.in
    interfaces/Integer.in
    interfaces/Long.in
    interfaces/Point.in
    interfaces/Rect.in
    interfaces/String.in
    interfaces/Vector.in
)

foreach (_file ${ANDROIDJNI_INTERFACES})
    if (NOT ANDROID)
        string(REGEX REPLACE "interfaces/(.*).in" "android/\\1.cpp" _source "${_file}")
        set(ANDROIDJNI_SOURCES ${ANDROIDJNI_SOURCES} ${_source})
    endif ()

    string(REGEX REPLACE "interfaces/(.*).in" "androidjni/\\1JNI.cpp" _source "${_file}")
    set(ANDROIDJNI_SOURCES ${ANDROIDJNI_SOURCES} ${_source})
endforeach ()

include_directories(
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/platforms/${TARGET_PLATFORM}"
    "${CMAKE_SOURCE_DIR}"
    "${CMAKE_CURRENT_BINARY_DIR}"
    "${CMAKE_CURRENT_BINARY_DIR}/GeneratedInterfaces"
    "${CMAKE_BINARY_DIR}"
)

add_custom_command(
    OUTPUT  ${LIBRARY_PRODUCT_DIR}/scripts/interface-generator.py
    MAIN_DEPENDENCY ${GENERATOR_SCRIPT}
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/scripts ${LIBRARY_PRODUCT_DIR}/scripts
    VERBATIM)

add_subdirectory(annotations)

add_definitions(-DBUILDING_JNILIB -DJNI_STATIC)

GENERATE_INTERFACE_STUBS(ANDROIDJNI_SOURCES "${ANDROIDJNI_INTERFACES}")
WRAP_SOURCELIST(${ANDROIDJNI_HEADERS} ${ANDROIDJNI_SOURCES})

add_library(androidjni++ ${ANDROIDJNI_HEADERS} ${ANDROIDJNI_SOURCES} ${GENERATOR_SCRIPT})

ADD_PREFIX_HEADER(androidjni++ JNIExportMacros.h)
ADD_POST_BUILD_COMMAND(androidjni++)

COPY_LIBRARY_HEADERS_DIRECTORY(androidjni++ android include/android)
COPY_LIBRARY_HEADERS_DIRECTORY(androidjni++ androidjni include/android/JNI)
COPY_LIBRARY_HEADERS_DIRECTORY(androidjni++ "${CMAKE_CURRENT_BINARY_DIR}/GeneratedInterfaces" include/android)
COPY_LIBRARY_HEADERS(androidjni++ "${ANDROIDJNI_HEADERS}" include/androidjni)
