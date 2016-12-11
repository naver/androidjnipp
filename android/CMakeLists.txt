set(ANDROID_SOURCES)

set(GENERATOR_INTERFACES_DIR "${CMAKE_SOURCE_DIR}/generator/interfaces")

set(ANDROID_INTERFACES
    ${GENERATOR_INTERFACES_DIR}/Boolean.in
    ${GENERATOR_INTERFACES_DIR}/HashMap.in
    ${GENERATOR_INTERFACES_DIR}/Integer.in
    ${GENERATOR_INTERFACES_DIR}/Long.in
    ${GENERATOR_INTERFACES_DIR}/Point.in
    ${GENERATOR_INTERFACES_DIR}/Rect.in
    ${GENERATOR_INTERFACES_DIR}/String.in
    ${GENERATOR_INTERFACES_DIR}/Vector.in
)

foreach (_file ${ANDROID_INTERFACES})
    if (NOT ANDROID)
        string(REGEX REPLACE "${GENERATOR_INTERFACES_DIR}/(.*).in" "\\1.cpp" _source "${_file}")
        set(ANDROID_SOURCES ${ANDROID_SOURCES} ${_source})
    endif ()

    string(REGEX REPLACE "${GENERATOR_INTERFACES_DIR}/(.*).in" "JNI/\\1JNI.cpp" _source "${_file}")
    set(ANDROID_SOURCES ${ANDROID_SOURCES} ${_source})
endforeach ()

include_directories(
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${CMAKE_SOURCE_DIR}"
    "${CMAKE_SOURCE_DIR}/androidjni"
    "${CMAKE_SOURCE_DIR}/androidjni/platforms/${TARGET_PLATFORM}"
    "${CMAKE_CURRENT_BINARY_DIR}"
    "${CMAKE_CURRENT_BINARY_DIR}/GeneratedFiles"
    "${CMAKE_BINARY_DIR}"
)

add_definitions(-DBUILDING_JNILIB -DJNI_STATIC)

GENERATE_INTERFACE_STUBS(ANDROID_SOURCES "${ANDROID_INTERFACES}")
WRAP_SOURCELIST(${ANDROID_HEADERS} ${ANDROID_SOURCES})

add_library(android OBJECT ${ANDROID_SOURCES})

ADD_PREFIX_HEADER(android JNIExportMacros.h)
