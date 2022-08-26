cmake_minimum_required(VERSION 3.22)
project(PipelineService)

set(CMAKE_CXX_STANDARD 14)


SET(CMAKE_BUILD_TYPE "Release")
#SET(CMAKE_BUILD_TYPE "Debug")

# 关闭警告
add_definitions(-w)

####################  Release生成pbd文件 ####################
#告诉编译器生成调试信息
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Zi")
#告诉链接器生成exe时加入调试信息
set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /DEBUG /OPT:REF /OPT:ICF")
#告诉链接器生成dll时加入调试信息
set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} /DEBUG /OPT:REF /OPT:ICF")

####################  设置输出目录 ####################
set(BUILD_DIR ${CMAKE_CURRENT_SOURCE_DIR}/build)
set(LIB_DIR ${BUILD_DIR}/Release)
if (CMAKE_BUILD_TYPE MATCHES "Debug")
    set(LIB_DIR ${BUILD_DIR}/Debug)
endif ()

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${LIB_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${LIB_DIR}/bin)
set(CMAKE_PDB_OUTPUT_DIRECTORY ${LIB_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${LIB_DIR}/bin)

####################  设置相关的include的目录 ####################
set(INC_DIR
        "E:/tools/windows/c++/thirdLib/LuaBridge-master/Source"
        "E:/tools/windows/lua/LuaJIT/src"

        "E:/tools/windows/c++/thirdLib/gdal/FileGDB_API_VS2019/include"
        "E:/tools/windows/vcpkg/vcpkg-2022.08.15/packages/gdal_x64-windows/include"
        "E:/tools/windows/vcpkg/vcpkg-2022.08.15/packages/geos_x64-windows/include"
        )
include_directories(${INC_DIR})

####################  设置相关的库的目录 ####################
#  不用vcpkg的gdal是因为, filegdb的debug版插件一直出错用不了
set(LINK_DIR
        "E:/tools/windows/lua/LuaJIT/src"

        "E:/tools/windows/cpp/thirdLib/gdal/FileGDB_API_VS2019/lib64"
        "E:/tools/windows/cpp/thirdLib/gdal/gdal-3.5.1"
        "E:/tools/windows/vcpkg/vcpkg-2022.08.15/packages/geos_x64-windows/debug/lib"
        "E:/tools/windows/vcpkg/vcpkg-2022.08.15/packages/geos_x64-windows/lib"
        )
link_directories(${LINK_DIR})

####################  获取所有.h .cpp文件，添加进去 ####################
file(GLOB HEAD_FILES
        src/*.h
        src/core/*.h
        src/service/*.h
        )
file(GLOB SOURCE_FILES
        src/*.cpp
        src/core/*.cpp
        src/test/*.cpp
        src/service/*.cpp
        )
add_executable(${PROJECT_NAME} ${HEAD_FILES} ${SOURCE_FILES})

####################### vcpkg package #################
find_package(fmt CONFIG REQUIRED)
find_package(Catch2 CONFIG REQUIRED)
find_package(yaml-cpp CONFIG REQUIRED)
find_package(log4cplus CONFIG REQUIRED)
find_path(CPP_HTTPLIB_INCLUDE_DIRS "httplib.h")

####################### vcpkg libs ####################
set(VCPKG_LIBRARY_DIRS PRIVATE
        fmt::fmt
        Catch2::Catch2 Catch2::Catch2WithMain
        yaml-cpp
        log4cplus::log4cplus
        ${CPP_HTTPLIB_INCLUDE_DIRS}
        )
####################  设置相关库文件 ####################
if (CMAKE_BUILD_TYPE MATCHES "Debug")
    target_link_libraries(${PROJECT_NAME}
            ${VCPKG_LIBRARY_DIRS}

            lua51.lib
            luajit.lib

            FileGDBAPID.lib
            geos_c.lib
            gdal_i_d.lib
            )
else ()
    target_link_libraries(${PROJECT_NAME}
            ${VCPKG_LIBRARY_DIRS}

            lua51.lib
            luajit.lib

            FileGDBAPI.lib
            geos_c.lib
            gdal_i.lib
            )
endif ()