set(TEST_SOURCES)

if (ANDROID)
    list(APPEND TEST_SOURCES
        android/AndroidManifest.xml
        android/stub.cpp

        android/src/com/example/test/StringGenerator.java
        android/src/com/example/test/StringGeneratorClient.java
        android/src/com/example/test/TestActivity.java
    )
    if (MSVC)
        set_source_files_properties(android/stub.cpp PROPERTIES HEADER_FILE_ONLY TRUE)
    endif ()

    set(CMAKE_ANDROID_GUI TRUE)
elseif (WIN32)
    list(APPEND TEST_SOURCES
        win32/WinMain.cpp
    )

    add_definitions(-DJNI_STATIC)
endif ()

include_directories(
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${LIBRARY_PRODUCT_DIR}/include/androidjni++"
    "${LIBRARY_PRODUCT_DIR}/include/androidjni++/android"
    "${CMAKE_SOURCE_DIR}"
    "${CMAKE_CURRENT_BINARY_DIR}"
    "${CMAKE_CURRENT_BINARY_DIR}/../testlib/GeneratedFiles"
    "${CMAKE_BINARY_DIR}"
)

add_executable(testapp ${TEST_SOURCES})

if (ANDROID)
    set_target_properties(testapp PROPERTIES ANDROID_JAR_DEPENDENCIES androidjni.annotations)
    import_libraries(testapp testlib)
    add_dependencies(testapp androidjni.annotations)
elseif (WIN32)
    target_link_libraries(testapp testlib)
endif ()
