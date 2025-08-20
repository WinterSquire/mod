cmake_minimum_required(VERSION 3.5)

set(CMAKE_CXX_STANDARD 20)

project(mod)

if (WIN32)
    set(CMAKE_DEBUG_POSTFIX d)
endif()

add_library(${PROJECT_NAME} STATIC
	${CMAKE_SOURCE_DIR}/src/assembly.cpp
	${CMAKE_SOURCE_DIR}/src/mod.cpp
    ${CMAKE_SOURCE_DIR}/dep/hde/hde64.c
)

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/dep/hde)

target_include_directories(${PROJECT_NAME} PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# install lib
install(TARGETS mod
        EXPORT modTargets
        LIBRARY DESTINATION lib
        INCLUDES DESTINATION include)

# install header 
install(DIRECTORY include/mod
        DESTINATION include)

# install config
include(CMakePackageConfigHelpers)
configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/modConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/modConfig.cmake"
    INSTALL_DESTINATION lib/cmake/mod
)

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/modConfig.cmake"
        DESTINATION lib/cmake/mod)

install(EXPORT modTargets
        FILE modTargets.cmake
        NAMESPACE mod::
        DESTINATION lib/cmake/mod)
    
# Test
find_package(GTest REQUIRED)

include(GoogleTest)

enable_testing()

add_executable(mod_test ${CMAKE_SOURCE_DIR}/test/test.cpp)

target_link_libraries(mod_test PRIVATE GTest::gtest mod)

gtest_discover_tests(mod_test)