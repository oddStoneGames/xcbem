cmake_minimum_required(VERSION 3.1)
set(CMAKE_C_STANDARD 11)

project(xcbem)

# Find XCB libraries and include directories
find_package(PkgConfig REQUIRED)
pkg_check_modules(XCB REQUIRED xcb)

# Create the Executable and link libraries
add_executable(${PROJECT_NAME} xcbem.c)
target_link_libraries(${PROJECT_NAME} PRIVATE ${XCB_LIBRARIES})

# Install the binary to /usr/local/bin
install(TARGETS ${PROJECT_NAME} DESTINATION /usr/local/bin)

# Create a uninstall target to remove the installed files
add_custom_target(uninstall COMMAND ${CMAKE_COMMAND} -E remove /usr/local/bin/${PROJECT_NAME})

# Define an alias to run `make uninstall`
add_custom_target(uninstall_target COMMENT "Uninstalling the program" COMMAND make uninstall)