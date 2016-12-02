cmake_minimum_required(VERSION 3.4)

project(androidjni++)

set(PYTHON python)
set(GENERATOR_SCRIPT "${CMAKE_SOURCE_DIR}/androidjni/scripts/interface-generator.py")

if (NOT LIBRARY_PRODUCT_DIR)
    set(LIBRARY_PRODUCT_DIR "${CMAKE_BINARY_DIR}" CACHE PATH "Path to the directory where products will be copied to.")
endif ()
get_filename_component(LIBRARY_PRODUCT_DIR ${LIBRARY_PRODUCT_DIR} ABSOLUTE)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${LIBRARY_PRODUCT_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${LIBRARY_PRODUCT_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${LIBRARY_PRODUCT_DIR})

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
include(HelperMacros)
include(Options${CMAKE_SYSTEM_NAME})

if (ANDROID)
    find_package(Java 1.7 EXACT)
    include(UseJava)

    set(CMAKE_ANDROID_API_MIN 14)
endif ()

if (ANDROID)
    set(TARGET_PLATFORM android)
else ()
    set(TARGET_PLATFORM generic)
endif ()

macro(LIST_INTERFACE_OUTPUTS _output_list _basename)
    set(_interface_outputs
        ${CMAKE_CURRENT_BINARY_DIR}/GeneratedInterfaces/${_basename}NativesStub.cpp
    )
    if (NOT ANDROID)
        set(_interface_outputs ${_interface_outputs}
            ${CMAKE_CURRENT_BINARY_DIR}/GeneratedInterfaces/${_basename}ManagedStub.cpp
        )
    endif ()

    set(${_output_list} ${${_output_list}} ${_interface_outputs})
    unset(_interface_outputs)
endmacro()

macro(GENERATE_INTERFACE_STUBS _output_sources _idl_list)
    foreach (_idl ${_idl_list})
        get_filename_component(_basename ${_idl} NAME_WE)
        get_filename_component(_absolute ${_idl} ABSOLUTE)
        list_interface_outputs(_outputs ${_basename})
        set(${_output_sources} ${${_output_sources}} ${_idl} ${_outputs})
        add_custom_command(
            OUTPUT  ${_outputs}
            MAIN_DEPENDENCY ${_idl}
            DEPENDS ${GENERATOR_SCRIPT}
            COMMAND ${PYTHON} ${GENERATOR_SCRIPT} --java ${_absolute} --shared ${CMAKE_CURRENT_BINARY_DIR}/GeneratedInterfaces --${TARGET_PLATFORM} ${CMAKE_CURRENT_BINARY_DIR}/GeneratedInterfaces
            VERBATIM)
        unset(_outputs)
    endforeach ()
endmacro()

add_subdirectory(androidjni)
add_subdirectory(test)
