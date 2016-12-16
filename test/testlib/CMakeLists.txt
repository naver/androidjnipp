set(TESTLIB_SOURCES
    StringGeneratorClientNatives.cpp
    StringGeneratorNatives.cpp
)

if (ANDROID)
    list(APPEND TESTLIB_SOURCES
        android/JNIMain.cpp
    )
else ()
    list(APPEND TESTLIB_SOURCES
        generic/StringGeneratorClient.cpp
        generic/StringGenerator.cpp
    )

    if (WIN32)
        list(APPEND TESTLIB_SOURCES
            win32/DLLMain.cpp
        )
    endif ()
endif ()

include_directories(
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${LIBRARY_PRODUCT_DIR}/include/androidjni++"
    "${LIBRARY_PRODUCT_DIR}/include/androidjni++/android"
    "${CMAKE_SOURCE_DIR}"
    "${CMAKE_CURRENT_BINARY_DIR}"
    "${CMAKE_CURRENT_BINARY_DIR}/GeneratedFiles"
    "${CMAKE_BINARY_DIR}"
)

link_directories(
    ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
)

set(TESTLIB_JAVA_SOURCES
    ../testapp/android/src/com/example/test/StringGenerator.java
    ../testapp/android/src/com/example/test/StringGeneratorClient.java
)

add_definitions(-DJNI_STATIC)

GENERATE_INTERFACE_STUBS(TESTLIB_SOURCES "${TESTLIB_JAVA_SOURCES}")
WRAP_SOURCELIST(${TESTLIB_SOURCES})

add_library(testlib SHARED ${TESTLIB_SOURCES})

ADD_PREFIX_HEADER(testlib TestLibPrefix.h)

target_link_libraries(testlib PUBLIC androidjni++)

if (ANDROID)
    target_link_libraries(testlib PRIVATE log)
endif ()
