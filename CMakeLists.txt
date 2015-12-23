cmake_minimum_required(VERSION 3.0)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

project("plcify")

include_directories(3rdparty/spandsp-0.0.6/src/spandsp)

add_executable(plcify code/plcify/main.c)

find_library(LibSndFile REQUIRED)
include_directories(${LibSndFile_INCLUDE_DIRS})
set(LIBS ${LIBS} ${LibSndFile_LIBRARIES})

find_library(Spandsp REQUIRED)
include_directories(${Spandsp_INCLUDE_DIRS})
set(LIBS ${LIBS} ${Spandsp_LIBRARIES})

target_link_libraries(plcify ${LIBS})