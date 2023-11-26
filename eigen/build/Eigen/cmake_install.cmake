# Install script for directory: /mnt/c/projects/eigen/Eigen

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Release")
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

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "1")
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

# Set default install directory permissions.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "/usr/bin/objdump")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xDevelx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/eigen3/Eigen" TYPE FILE FILES
    "/mnt/c/projects/eigen/Eigen/Cholesky"
    "/mnt/c/projects/eigen/Eigen/CholmodSupport"
    "/mnt/c/projects/eigen/Eigen/Core"
    "/mnt/c/projects/eigen/Eigen/Dense"
    "/mnt/c/projects/eigen/Eigen/Eigen"
    "/mnt/c/projects/eigen/Eigen/Eigenvalues"
    "/mnt/c/projects/eigen/Eigen/Geometry"
    "/mnt/c/projects/eigen/Eigen/Householder"
    "/mnt/c/projects/eigen/Eigen/IterativeLinearSolvers"
    "/mnt/c/projects/eigen/Eigen/Jacobi"
    "/mnt/c/projects/eigen/Eigen/LU"
    "/mnt/c/projects/eigen/Eigen/MetisSupport"
    "/mnt/c/projects/eigen/Eigen/OrderingMethods"
    "/mnt/c/projects/eigen/Eigen/PaStiXSupport"
    "/mnt/c/projects/eigen/Eigen/PardisoSupport"
    "/mnt/c/projects/eigen/Eigen/QR"
    "/mnt/c/projects/eigen/Eigen/QtAlignedMalloc"
    "/mnt/c/projects/eigen/Eigen/SPQRSupport"
    "/mnt/c/projects/eigen/Eigen/SVD"
    "/mnt/c/projects/eigen/Eigen/Sparse"
    "/mnt/c/projects/eigen/Eigen/SparseCholesky"
    "/mnt/c/projects/eigen/Eigen/SparseCore"
    "/mnt/c/projects/eigen/Eigen/SparseLU"
    "/mnt/c/projects/eigen/Eigen/SparseQR"
    "/mnt/c/projects/eigen/Eigen/StdDeque"
    "/mnt/c/projects/eigen/Eigen/StdList"
    "/mnt/c/projects/eigen/Eigen/StdVector"
    "/mnt/c/projects/eigen/Eigen/SuperLUSupport"
    "/mnt/c/projects/eigen/Eigen/UmfPackSupport"
    )
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xDevelx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/eigen3/Eigen" TYPE DIRECTORY FILES "/mnt/c/projects/eigen/Eigen/src" FILES_MATCHING REGEX "/[^/]*\\.h$")
endif()

