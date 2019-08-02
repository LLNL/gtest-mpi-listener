# TODO(oxberry1@llnl.gov): Add banner comment here

cmake_minimum_required(VERSION 2.8.11)

project(gtest-mpi-listener
  LANGUAGES C CXX)

# Download and unpack googletest at configure time
configure_file(vendor-googletest.cmake.in googletest-download/CMakeLists.txt)
execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
  RESULT_VARIABLE result
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download )
if(result)
  message(FATAL_ERROR "CMake step for googletest failed: ${result}")
endif()
execute_process(COMMAND ${CMAKE_COMMAND} --build .
  RESULT_VARIABLE result
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download )
if(result)
  message(FATAL_ERROR "Build step for googletest failed: ${result}")
endif()

# Prevent overriding the parent project's compiler/linker
# settings on Windows
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

# Add googletest directly to our build. This defines
# the gtest and gtest_main targets.
add_subdirectory(${CMAKE_CURRENT_BINARY_DIR}/googletest-src
                 ${CMAKE_CURRENT_BINARY_DIR}/googletest-build
                 EXCLUDE_FROM_ALL)

# The gtest/gtest_main targets carry header search path
# dependencies automatically when using CMake 2.8.11 or
# later. Otherwise we have to add them here ourselves.
if (CMAKE_VERSION VERSION_LESS 2.8.11)
  include_directories("${gtest_SOURCE_DIR}/include")
endif()

# In FindMPI.cmake, "CXX" refers to "the MPI C API [being] usable from C++"
find_package(MPI 1.2 REQUIRED)

# Add executable
add_executable(example src/example.cpp)
target_link_libraries(example PUBLIC gtest ${MPI_C_LINK_FLAGS}
  ${MPI_C_LIBRARIES} MPI::MPI_C)
target_include_directories(example PUBLIC include ${MPI_C_INCLUDE_DIRS})

target_compile_features(gtest PUBLIC cxx_std_11)
