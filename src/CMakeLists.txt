cmake_minimum_required(VERSION 3.2)
project(YSFLIGHT)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (MSVC)
  add_compile_definitions(
    _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES
    _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES_COUNT
    _CRT_SECURE_NO_WARNINGS
  )
endif()

if(NOT "${CMAKE_SYSTEM_NAME}" STREQUAL "WindowsStore" AND NOT ANDROID)
	set(BUILD_YSFLIGHT_CONSOLE_SERVER 1)
endif()



if("${CMAKE_SYSTEM_NAME}" STREQUAL "WindowsStore")
	add_definitions(-DYS_IS_UNIVERSAL_WINDOWS_APP)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /ZW")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /ZW")
endif()
set(CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_SOURCE_DIR}/vcpkg.json)


include_directories(${CMAKE_CURRENT_SOURCE_DIR})

if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/../ysce_public/src)
	add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../ysce_public/src public)
else()
	add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../ysce_public/src public)
endif()

add_subdirectory(common)
add_subdirectory(config)
add_subdirectory(platform/common)
add_subdirectory(filename)
add_subdirectory(dynamics)
add_subdirectory(autopilot)
add_subdirectory(pathplanning)
add_subdirectory(externalconsole)
add_subdirectory(util)
add_subdirectory(vehicle)
add_subdirectory(scenery)
add_subdirectory(ysjoystick/src)
set(ysflight_filename_INCLUDE_DIR ${ysflight_filename_INCLUDE_DIR} PARENT_SCOPE)

add_subdirectory(graphics/common)
add_subdirectory(graphics/gl)
add_subdirectory(graphics/gl2.0)
add_subdirectory(graphics/null)

if(ysd3d9_EXIST)
	add_subdirectory(graphics/d3d9)
endif()

if(MSVC)
	add_subdirectory(platform/win32)
	add_subdirectory(sounddll/win32)
	add_subdirectory(voicedll/generic)
elseif(ANDROID)
	add_subdirectory(platform/android)
elseif(APPLE)
	add_subdirectory(platform/macosx)
	add_subdirectory(sounddll/macosx)
	add_subdirectory(voicedll/generic)
elseif(UNIX)
	add_subdirectory(platform/linux)
	add_subdirectory(sounddll/linux-alsa)
	add_subdirectory(sounddll/linux-pulseaudio)
	add_subdirectory(voicedll/generic)
else()
	add_subdirectory(voicedll/generic)
endif()


if(BUILD_YSFLIGHT_CONSOLE_SERVER)
	add_subdirectory(platform/nownd)
endif()


add_subdirectory(core)
add_subdirectory(ui)
add_subdirectory(gui)

add_subdirectory(main)
add_subdirectory(main_consvr)
add_subdirectory(main_demoOnly)
