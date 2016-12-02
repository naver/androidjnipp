add_subdirectory(testapp)
add_subdirectory(testlib)

add_dependencies(testlib androidjni++)
add_dependencies(testapp testlib)
