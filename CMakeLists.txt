cmake_minimum_required(VERSION 3.10)
project(rabbitmq-sample)

set(CMAKE_GENERATOR "Ninja")

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# CUDA support
find_package(CUDA REQUIRED)
enable_language(CUDA)

# 最適化オプションの設定
#if(NOT CMAKE_BUILD_TYPE)
#    set(CMAKE_BUILD_TYPE Release)
#endif()

# Set compiler flags for GCC
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -march=native -mtune=native")
endif()

# Find Open3D with GPU support
# Set the path to the Open3D library with GPU support
set(Open3D_DIR "/usr/local/include/open3d_cuda/lib/cmake/Open3D")
find_package(Open3D REQUIRED)
find_package(Boost REQUIRED COMPONENTS system filesystem)
find_package(laserpants_dotenv)
find_package(jsoncpp REQUIRED)
find_library(LIBRABBITMQ NAMES rabbitmq REQUIRED)
find_library(LIBSIMPLEAMQPCLIENT NAMES SimpleAmqpClient REQUIRED)
find_library(LIBJSON NAMES jsoncpp REQUIRED)

# Find Open3D, and you might need to set Open3D_DIR variable
# to the location of Open3DConfig.cmake file via -DOpen3D_DIR=...
find_package(Open3D REQUIRED)
find_package(Boost REQUIRED COMPONENTS system filesystem)
# find_package(PCL REQUIRED)

# Add executable
add_executable(main 
  src/main.cpp 
)

# Link Open3D libraries
target_link_libraries(main
  ${Open3D_LIBRARIES}
  Boost::system
  Boost::filesystem
  ${CUDA_LIBRARIES}
  # CUDA::cudart # Link CUDA runtime library
  laserpants::dotenv
#   ${PCL_LIBRARIES}
  jsoncpp_lib
  ${LIBSIMPLEAMQPCLIENT} 
  ${LIBRABBITMQ} 
  ${LIBJSON} 
  Threads::Threads
)

# Include directories
target_include_directories(main PRIVATE
  ${Open3D_INCLUDE_DIRS}
  ${CUDA_INCLUDE_DIRS}
  ${JSONCPP_INCLUDE_DIRS}
)

# Additional configurations for linking
target_compile_definitions(main PRIVATE ${Open3D_DEFINITIONS})

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# カスタムクリーンターゲットの追加
add_custom_target(custom_clean
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_BINARY_DIR}/clean.cmake
)

# clean.cmakeファイルの作成
file(WRITE ${CMAKE_BINARY_DIR}/clean.cmake
"file(REMOVE_RECURSE client server http3-client http3-server client_main server_main build *.dSYM)")