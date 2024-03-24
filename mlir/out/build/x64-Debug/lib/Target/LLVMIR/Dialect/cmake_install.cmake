# Install script for directory: D:/llvm/llvm-nnsu-2024/mlir/lib/Target/LLVMIR/Dialect

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "D:/llvm/llvm-nnsu-2024/mlir/out/install/x64-Debug")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Debug")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("D:/llvm/llvm-nnsu-2024/mlir/out/build/x64-Debug/lib/Target/LLVMIR/Dialect/ArmNeon/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("D:/llvm/llvm-nnsu-2024/mlir/out/build/x64-Debug/lib/Target/LLVMIR/Dialect/ArmSME/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("D:/llvm/llvm-nnsu-2024/mlir/out/build/x64-Debug/lib/Target/LLVMIR/Dialect/ArmSVE/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("D:/llvm/llvm-nnsu-2024/mlir/out/build/x64-Debug/lib/Target/LLVMIR/Dialect/AMX/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("D:/llvm/llvm-nnsu-2024/mlir/out/build/x64-Debug/lib/Target/LLVMIR/Dialect/Builtin/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("D:/llvm/llvm-nnsu-2024/mlir/out/build/x64-Debug/lib/Target/LLVMIR/Dialect/GPU/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("D:/llvm/llvm-nnsu-2024/mlir/out/build/x64-Debug/lib/Target/LLVMIR/Dialect/LLVMIR/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("D:/llvm/llvm-nnsu-2024/mlir/out/build/x64-Debug/lib/Target/LLVMIR/Dialect/NVVM/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("D:/llvm/llvm-nnsu-2024/mlir/out/build/x64-Debug/lib/Target/LLVMIR/Dialect/OpenACC/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("D:/llvm/llvm-nnsu-2024/mlir/out/build/x64-Debug/lib/Target/LLVMIR/Dialect/OpenMP/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("D:/llvm/llvm-nnsu-2024/mlir/out/build/x64-Debug/lib/Target/LLVMIR/Dialect/ROCDL/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("D:/llvm/llvm-nnsu-2024/mlir/out/build/x64-Debug/lib/Target/LLVMIR/Dialect/X86Vector/cmake_install.cmake")
endif()

