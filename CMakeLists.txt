cmake_minimum_required(VERSION 3.6.2)
project(installer)


set(CMAKE_CXX_STANDARD 14)
include_directories(.)
IF (WIN32)
    include_directories(H:/common/boost_1_67_0)
ELSE()
    include_directories(/usr/local/include/)
ENDIF()

add_executable(installer main.cpp Helpers/ProgressBar.h Install/Install.h Install/Head.h Install/Items.h Install/Screens.h Install/Download.h Jobs/ProgressJob.h Install/Settings.h)