set(TARGET_NAME ysjoystick)
set(LIB_DEPENDENCY ysport ysfontrenderer fsguilib ysbitmap ysbitmapfont)

# Don't link fssimplewindow.  It can be fssimplenowindow.  Just add include directory
get_property(ADDITIONAL_INCLUDE_DIR1 TARGET fssimplewindow PROPERTY INCLUDE_DIRECTORIES)
get_property(ADDITIONAL_INCLUDE_DIR2 TARGET yssystemfont PROPERTY INCLUDE_DIRECTORIES)



if(APPLE)
	find_library(IOKIT_LIB IOKit)
	mark_as_advanced(IOKIT_LIB)
	set(LIB_DEPENDENCY ${LIB_DEPENDENCY} ${IOKIT_LIB})
endif()



if(MSVC)
	set(platform_SRCS windows/ysjoyreader.cpp)
	include_directories(windows)
	set(platform_HEADERS )
elseif(APPLE)
	set(platform_SRCS macosx/ysjoyreader-objc.m macosx/ysjoyreader.cpp)
	include_directories(macosx)
	set(platform_HEADERS )
elseif(UNIX)
	set(platform_SRCS linux/ysjoyreader.cpp)
	include_directories(linux)
	set(platform_HEADERS )
else()
	set(platform_SRCS null/ysjoyreader.cpp)
	set(platform_HEADERS "")
endif()



set(SRCS
${platform_SRCS}
ysjoyreadercommon.cpp
)

set(HEADERS
ysjoyreader.h
${platform_HEADERS}
)

add_library(${TARGET_NAME} ${SRCS})
target_include_directories(${TARGET_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${ADDITIONAL_INCLUDE_DIR1} ${ADDITIONAL_INCLUDE_DIR2})
target_link_libraries(${TARGET_NAME} ${LIB_DEPENDENCY})

if(NOT ANDROID)
	add_library(ysjoycalibdlg_gl1 fsjoycalibdlggl.cpp fsjoycalibdlg.cpp)
	target_include_directories(ysjoycalibdlg_gl1 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${ADDITIONAL_INCLUDE_DIR1} ${ADDITIONAL_INCLUDE_DIR2})
	target_link_libraries(ysjoycalibdlg_gl1 ysjoystick)
endif()

add_library(ysjoycalibdlg_gl2 fsjoycalibdlggl2.0.cpp fsjoycalibdlg.cpp)
target_include_directories(ysjoycalibdlg_gl2 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${ADDITIONAL_INCLUDE_DIR1} ${ADDITIONAL_INCLUDE_DIR2})
target_link_libraries(ysjoycalibdlg_gl2 ysjoystick)
