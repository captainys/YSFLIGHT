if(CMAKE_SIZEOF_VOID_P EQUAL 8)
	set(BITNESS 64)
else()
	set(BITNESS 32)
endif()



# Windows -> gl1, gl2, d3d9, nownd
# Mac OSX -> gl1, gl2
# Linux   -> gl1, gl2, nownd
# Android -> gl2

if("${CMAKE_SYSTEM_NAME}" STREQUAL "WindowsStore")
	set(TARGET_NAME ysflight${BITNESS}_gl2demo)
elseif(MSVC)
	set(TARGET_NAME ysflight${BITNESS}_gl2demo ysflight${BITNESS}_gl1demo)
elseif(APPLE)
	set(TARGET_NAME ysflight${BITNESS}_gl1demo ysflight${BITNESS}_gl2demo)
elseif(ANDROID)
	set(TARGET_NAME ysflight${BITNESS}_gl2demo)
elseif(UNIX)
	set(TARGET_NAME ysflight${BITNESS}_gl1demo ysflight${BITNESS}_gl2demo)
else()
	message(FATAL_ERROR "Unsupported platform.")
endif()



set(VERBOSE_MODE 0)
set(IS_LIBRARY_PROJECT 0)
set(LIB_DEPENDENCY 
geblkernel
geblgl
ysflight_ui
ysflight_gui
ysflight_common
ysflight_core
ysflight_pathplanning 
ysflight_externalconsole 
ysflight_autopilot 
ysflight_dynamics
ysflight_vehicle 
ysflight_util
yssocket
ysflight_graphics_common
fsguilib
fsguifiledialog
ysbitmap
ysbitmapfont
ysfontrenderer
ysport
ysscenery_dnm
ystexturemanager
ysflight_filename
ysclass
ysclass11
ysglcpp
)
set(INCLUDE_DEPENDENCY)
set(OWN_HEADER_PATH .)
set(ADDITIONAL_HEADER_PATH)
set(SINGLE_TARGET 0)
set(SUB_FOLDER "YSFLIGHT")
set(REALLY_NEED_CPP11 1)    # Setting 1 will enable use of C++11 libraries, but lose compatibility with OSX 10.6.
set(WIN_SUBSYSTEM WINDOWS)
set(DATA_FILE_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/../../runtime")


if(APPLE)
	set(EXE_TYPE MACOSX_BUNDLE)
elseif("${CMAKE_SYSTEM_NAME}" STREQUAL "WindowsStore")
	set(EXE_TYPE)
elseif(MSVC)
	set(EXE_TYPE)
else()
	set(EXE_TYPE)
endif()


if(ysd3d9_EXIST)
	set(INCLUDE_DEPENDENCY ${INCLUDE_DEPENDENCY} ysd3d)
endif()



set(YSFLIGHT_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}")




#YSBEGIN "CMake Header" Ver 20170110
# YS CMakeLists Template
# Copyright (c) 2015 Soji Yamakawa.  All rights reserved.
# http://www.ysflight.com
# 
# Redistribution and use in source and binary forms, with or without modification, 
# are permitted provided that the following conditions are met:
# 
# 1. Redistributions of source code must retain the above copyright notice, 
#    this list of conditions and the following disclaimer.
# 
# 2. Redistributions in binary form must reproduce the above copyright notice, 
#    this list of conditions and the following disclaimer in the documentation 
#    and/or other materials provided with the distribution.
# 
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, 
# THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR 
# PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS 
# BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE 
# GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) 
# HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT 
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT 
# OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

cmake_minimum_required(VERSION 3.0.0)
if("${CMAKE_CURRENT_SOURCE_DIR}" MATCHES "^${CMAKE_SOURCE_DIR}" AND
   "${CMAKE_BINARY_DIR}" MATCHES "^${CMAKE_SOURCE_DIR}")
	message(FATAL_ERROR "In-source build prohibited.\nClear cache and Start cmake from somewhere else.")
	# First condition is to allow inclusion of the project from outside CMake project with
	# explicit binary-directory specification.   eg. add_subdirectory from Android CMakeLists.txt
endif()

if(MSVC)
	if(NOT WIN_SUBSYSTEM)
		set(WIN_SUBSYSTEM CONSOLE)
	endif()

	if("${CMAKE_SYSTEM_NAME}" STREQUAL "WindowsStore")
		if(EXCLUDE_IN_UNIVERSAL_WINDOWS EQUAL 1)
			return()
		endif()

		add_definitions(-DYS_IS_UNIVERSAL_WINDOWS_APP)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /ZW")
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /ZW")
	endif()

	# I want to keep compatibility with older operating systems, but it's getting difficult.
	# I have to comment out the following lines.
	# if(CMAKE_SIZEOF_VOID_P EQUAL 8)
	# 	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:${WIN_SUBSYSTEM},5.02 /MACHINE:x64")
	# else()
	# 	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:${WIN_SUBSYSTEM},5.01 /MACHINE:X86")
	# endif()
endif()

if(NOT DEFINED TARGET_NAME)
	message(FATAL_ERROR "TARGET_NAME not defined.")
endif()
if(NOT DEFINED IS_LIBRARY_PROJECT)
	message(FATAL_ERROR "IS_LIBRARY_PROJECT not defined.")
endif()
if(NOT DEFINED SINGLE_TARGET)
	message(FATAL_ERROR "SINGLE_TARGET not defined.")
endif()

# 2016/09/22 Learned a better way than specifying -std=c++11
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
	# 2016/07/22
	#  /MT flags should be set outside the public repository.  It is moved to the higher-level CMakeLists.txt
elseif(APPLE)
	# 2015/07/15
	#   Sorry.  I pulled the plug.  All of my programs, including YS FLIGHT SIMULATOR, won't support 
	#   OSX 10.6 after today.  Apple deliberately disabled C++11 features in the libraries that I need to make my 
	#	programs compatible with OSX 10.6.
	#
	#	I know OSX 10.9 is evil for older models.  My 2008 MacBook Pro flies with OSX 10.6, but becoes
	#	a sloth with OSX 10.9.  Apple used to be a challenger pursuing Microsoft, but it is now an empire
	#	that Microsoft once was, and is doing everything that Microsoft did.  Apple inprison programmers
	#	with Apple-only programming language called Swift (already doing with Objective-C though) and Apple-only
	#	graphics toolkit called Metal, just as Microsoft did with C# and Direct3D.  Apple is making operating
	#	system heavier, slower, and inefficient, just as Microsoft has been doing.  The same thing is going all 
	#	around again.
	#
	#	OK, I warn you.  If you are investing your precious time for learning Swift and/or Metal, you are 
	#	taking a very big gamble.  Apple will throw it away when they get bored of it.  Learning one programming 
	#	language is not just understanding syntax.  You need to write considerable amount of code to learn the 
	#	best practices.  So far, C and C++ have been with for more than 20 years.  Will Swift live that long?
	#	Nobody knows.  I doubt it.  Swift is developed by a closed group.  Maybe one genius is in charge now.
	#	But, when the genius leaves, it could cramble down.  C and C++ are developed by the top computer
	#	scientists of the world.  To me, which is superior is obvious.
	#
	#	No user wants a new operating system.  Everyone wants their system to be cleaner, more stable, more 
	#	secure, and more resource-efficient.  Neither Apple nor Microsoft gets it.  We continue to be forced
	#	to throw away perfectly healthy hardware, and buy new over-spec hardware, which is inefficiently
	#	operated by the wasteful operating systems.
	#
	#	Sad and outrageous.  But, that's what Apple do.  Apple takes C++11 hostage and forces programmers 
	#	to drop support for older but still active-duty operating systems.
	#
	#	Mac is a good computer though.  I am happy with my 2011 MacMini.  I probably would be happy with
	#	my 2008 MacBook Pro if I still can (practically) use it with OSX 10.6, or if 10.9 is as efficient 
	#	as 10.6.

	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mmacosx-version-min=10.9 -Wno-switch")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mmacosx-version-min=10.9 -Wno-switch")
elseif(UNIX)
	# -Wl,--no-as-needed required for g++ 4.8.4 Confirmed unnecessary with 5.4.0
	#  http://stackoverflow.com/questions/19463602/compiling-multithread-code-with-g
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wl,--no-as-needed")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wl,--no-as-needed")
else()
endif()

if(IS_LIBRARY_PROJECT)
	#set(YS_LIBRARY_LIST ${YS_LIBRARY_LIST} ${TARGET_NAME} PARENT_SCOPE)
	# Modified as suggested in CMake performance tips.
	list(APPEND YS_LIBRARY_LIST ${TARGET_NAME})
	set(YS_LIBRARY_LIST ${YS_LIBRARY_LIST} PARENT_SCOPE)
endif()

#YSEND



if(MSVC)
	set(OS_LIBRARIES 
		kernel32.lib 
		user32.lib 
		comdlg32.lib 
		urlmon.lib 
		gdi32.lib 
		winmm.lib 
		ddraw.lib 
		advapi32.lib 
		wsock32.lib 
		shell32.lib 
		opengl32.lib 
		glu32.lib 
		opengl32.lib 
		glu32.lib
	)
	set(platform_SRCS hotasrc.rc)
	set(platform_HEADERS "")
elseif(APPLE)
	set(OS_LIBRARIES 
		""
	)
	set(platform_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/../platform/macosx/APPL.icns)
	set_source_files_properties(../platform/macosx/APPL.icns PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
	set(platform_HEADERS "")
elseif(UNIX)
	set(OS_LIBRARIES 
		""
	)
	set(platform_SRCS "")
	set(platform_HEADERS "")
else()
	set(OS_LIBRARIES 
		""
	)
	set(platform_SRCS "")
	set(platform_HEADERS "")
endif()




#tentative
set(SRCS
${platform_SRCS}
)

set(HEADERS
${platform_HEADERS}
resource.h
)


if(ysd3d9_EXIST)
	add_executable(ysflight${BITNESS}_d3d9demo fsmain.cpp ${SRCS})
	target_link_libraries(ysflight${BITNESS}_d3d9demo 
		ysflight_platform_win32 
		ysflight_platform_win32_d3d9 
		ysscenery_dnm_d3d 
		ystexturemanager_d3d9
		fsguilib_d3d9
		geblgl_d3d9
		ysglcpp_d3d9
		fslazywindow
		${OS_LIBRARIES} 
	)
	# d3d9.lib will be linked from ysd3d9.lib
endif()


if(NOT ANDROID)
	add_executable(ysflight${BITNESS}_gl2demo ${EXE_TYPE} fsmain.cpp ${SRCS})
	add_executable(ysflight${BITNESS}_gl1demo ${EXE_TYPE} fsmain.cpp ${SRCS})
else()
	add_library(ysflight${BITNESS}_gl2demo ${EXE_TYPE} fsmain.cpp ${SRCS})
endif()



#YSBEGIN "CMake Footer" Ver 20170110
if(YS_CXX_FLAGS)
	foreach(SRC ${SRCS})
		if(${SRC} MATCHES .cpp$)
			set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/${SRC} PROPERTIES COMPILE_FLAGS "${YS_CXX_FLAGS}")
		endif()
	endforeach(SRC)
endif()

# When template sources are unavoidable >>
if("${CMAKE_SYSTEM_NAME}" STREQUAL "WindowsStore" AND NOT IS_LIBRARY_PROJECT)
	get_property(XAML_TEMPLATE_DIR TARGET fslazywindow PROPERTY FS_XAML_TEMPLATE_DIR)
	get_property(XAML_ASSET_FILES TARGET fslazywindow PROPERTY FS_XAML_ASSET_FILES)
	get_property(XAML_APP_DEF_SOURCE TARGET fslazywindow PROPERTY FS_XAML_APP_DEF_SOURCE)
	get_property(XAML_CLATTER_SOURCE TARGET fslazywindow PROPERTY FS_XAML_CLATTER_SOURCE)
	get_property(XAML_PER_PROJ_SOURCE TARGET fslazywindow PROPERTY FS_XAML_PER_PROJ_SOURCE)
	foreach(SRC ${XAML_PER_PROJ_SOURCE})
		file(COPY ${XAML_TEMPLATE_DIR}/${SRC} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
		list(APPEND COPIED_XAML_PER_PROJ_SOURCE ${CMAKE_CURRENT_BINARY_DIR}/${SRC})
	endforeach(SRC)
	list(APPEND SRCS ${XAML_APP_DEF_SOURCE} ${XAML_CLATTER_SOURCE} ${COPIED_XAML_PER_PROJ_SOURCE} ${XAML_ASSET_FILES})
	include_directories(${XAML_TEMPLATE_DIR})
	set_source_files_properties(${XAML_ASSET_FILES} PROPERTIES VS_DEPLOYMENT_CONTENT 1)
	set_source_files_properties(${XAML_ASSET_FILES} PROPERTIES VS_DEPLOYMENT_LOCATION "Assets")
	set_source_files_properties(${XAML_APP_DEF_SOURCE} PROPERTIES VS_XAML_TYPE ApplicationDefinition)
endif()
# When template sources are unavoidable <<

foreach(ONE_TARGET ${TARGET_NAME})
	message([${ONE_TARGET}])

	if(SINGLE_TARGET)
		if(NOT IS_LIBRARY_PROJECT)
			add_executable(${ONE_TARGET} ${EXE_TYPE} ${SRCS} ${HEADERS})
		else()
			add_library(${ONE_TARGET} ${LIB_OPTION} ${SRCS} ${HEADERS})
		endif()
	endif()

	if(NOT IS_LIBRARY_PROJECT)
		if(EXE_COPY_DIR)
			# 2015/02/01 CMAKE_CONFIGURATION_TYPES may be empty.
			set_target_properties(${ONE_TARGET} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${EXE_COPY_DIR}")
			set_target_properties(${ONE_TARGET} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG "${EXE_COPY_DIR}")
			set_target_properties(${ONE_TARGET} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE "${EXE_COPY_DIR}")
			foreach(CFGTYPE ${CMAKE_CONFIGURATION_TYPES})
				string(TOUPPER ${CFGTYPE} UCFGTYPE)
				set_target_properties(${ONE_TARGET} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_${UCFGTYPE} "${EXE_COPY_DIR}")
			endforeach(CFGTYPE)
		endif()
	else()
		set(INHERITING_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}" ${OWN_HEADER_PATH} ${ADDITIONAL_HEADER_PATH})

		foreach(DEPEND_TARGET ${INCLUDE_DEPENDENCY})
			get_property(TARGET_INCLUDE_DIR TARGET ${DEPEND_TARGET} PROPERTY INCLUDE_DIRECTORIES)
			list(APPEND INHERITING_INCLUDE_DIR ${TARGET_INCLUDE_DIR})
		endforeach(DEPEND_TARGET)

		list(REMOVE_DUPLICATES INHERITING_INCLUDE_DIR)
		target_include_directories(${ONE_TARGET} PUBLIC ${INHERITING_INCLUDE_DIR})

		if(VERBOSE_MODE)
			message("Inheriting include directories ${INHERITING_INCLUDE_DIR}")
		endif()
	endif()

	set(${ONE_TARGET}_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}" PARENT_SCOPE)

	if(SUB_FOLDER)
		if(VERBOSE_MODE)
			message("Putting in folder ${SUB_FOLDER}")
		endif()
		set_property(TARGET ${ONE_TARGET} PROPERTY FOLDER ${SUB_FOLDER})
	endif()

	if(VERBOSE_MODE)
		foreach(LINKLIB ${LIB_DEPENDENCY})
			message(Lib=${LINKLIB})
		endforeach(LINKLIB)
	endif()
	target_link_libraries(${ONE_TARGET} ${LIB_DEPENDENCY})

	# We suffered enough from the shared stdc++
	if(UNIX AND NOT APPLE AND NOT "${CMAKE_SYSTEM_NAME}" STREQUAL "Android")
		target_link_libraries(${ONE_TARGET} pthread -static-libstdc++ -static-libgcc)
	endif()

	if(ADDITIONAL_HEADER_PATH)
		if(VERBOSE_MODE)
			message(Additional Include=${ADDITIONAL_HEADER_PATH})
		endif()
		include_directories(${ADDITIONAL_HEADER_PATH})
	endif()
endforeach(ONE_TARGET)

if(DATA_FILE_LOCATION)
	foreach(ONE_DATA_FILE_LOCATION ${DATA_FILE_LOCATION})
		foreach(ONE_TARGET ${TARGET_NAME})
			get_property(IS_MACOSX_BUNDLE TARGET ${ONE_TARGET} PROPERTY MACOSX_BUNDLE)

			if(DATA_COPY_DIR)
				set(DATA_DESTINATION ${DATA_COPY_DIR})
			else()
				if("${CMAKE_SYSTEM_NAME}" STREQUAL "Android")
					if(NOT YS_ANDROID_ASSET_DIRECTORY)
						MESSAGE(FATAL_ERROR "YS_ANDROID_ASSET_DIRECTORY not defined or empty.")
					endif()
					set(DATA_DESTINATION ${YS_ANDROID_ASSET_DIRECTORY})
				elseif(NOT EXE_COPY_DIR)
					if(APPLE AND IS_MACOSX_BUNDLE)
						set(DATA_DESTINATION "$<TARGET_FILE_DIR:${ONE_TARGET}>/../Resources")
					elseif("${CMAKE_SYSTEM_NAME}" STREQUAL "WindowsStore")
						set(DATA_DESTINATION "$<TARGET_FILE_DIR:${ONE_TARGET}>/Assets")
					elseif(MSVC)
						set(DATA_DESTINATION "$<TARGET_FILE_DIR:${ONE_TARGET}>")
					else()
						set(DATA_DESTINATION "$<TARGET_FILE_DIR:${ONE_TARGET}>")
					endif()
				else()
					if(IS_MACOSX_BUNDLE)
						set(DATA_DESTINATION "${EXE_COPY_DIR}/${ONE_TARGET}.app/Contents/Resources")
					elseif("${CMAKE_SYSTEM_NAME}" STREQUAL "WindowsStore")
						set(DATA_DESTINATION "${EXE_COPY_DIR}/Assets")
					else()
						set(DATA_DESTINATION "${EXE_COPY_DIR}")
					endif()
				endif()
			endif()

			# 2016/02/13 Use of generator-expression causes / be used in the DATA_DESTINATION
			#            What's worse is it is not replaced with \\ by REGEX because it
			#            is expanded at build time, not cmake time.
			#if(MSVC)
			#	string(REGEX REPLACE "/" "\\\\" WIN_ONE_DATA_FILE_LOCATION "${ONE_DATA_FILE_LOCATION}")
			#	string(REGEX REPLACE "/" "\\\\" WIN_DATA_DESTINATION "${DATA_DESTINATION}")
			#	add_custom_command(TARGET ${ONE_TARGET} POST_BUILD 
			#		COMMAND echo [File Copy]
			#		COMMAND echo From: "${WIN_ONE_DATA_FILE_LOCATION}\\*"
			#		COMMAND echo To:   "${WIN_DATA_DESTINATION}\\."
			#		COMMAND xcopy "${WIN_ONE_DATA_FILE_LOCATION}\\*" "${WIN_DATA_DESTINATION}\\." /E /D /C /Y
			#	)
			#else()
			#	add_custom_command(TARGET ${ONE_TARGET} POST_BUILD 
			#		COMMAND echo [File Copy]
			#		COMMAND echo From: "${ONE_DATA_FILE_LOCATION}"
			#		COMMAND echo To:   "${DATA_DESTINATION}"
			#		COMMAND mkdir -p "${DATA_DESTINATION}"
			#		COMMAND rsync -r "${ONE_DATA_FILE_LOCATION}/*" "${DATA_DESTINATION}"
			#	)
			#endif()

			# "cmake -E copy_directory" does the job in any cmake-supporting platforms, but what if the command-line cmake is not installed like MacOSX App?
			# 2016/02/13  Probably using ${CMAKE_COMMAND} is the solution.
			set_property(TARGET ${ONE_TARGET} PROPERTY YS_DATA_COPY_DIR "${DATA_DESTINATION}")
			add_custom_command(TARGET ${ONE_TARGET} POST_BUILD 
				COMMAND echo For:  ${ONE_TARGET}
				COMMAND echo Copy
				COMMAND echo From: ${ONE_DATA_FILE_LOCATION}
				COMMAND echo To:   ${DATA_DESTINATION}
				COMMAND "${CMAKE_COMMAND}" -E make_directory \"${DATA_DESTINATION}\"
				COMMAND "${CMAKE_COMMAND}" -E copy_directory \"${ONE_DATA_FILE_LOCATION}\" \"${DATA_DESTINATION}\")

		endforeach(ONE_TARGET)
	endforeach(ONE_DATA_FILE_LOCATION)
endif()

#YSEND



set(DLL_PROJS "")
set(NEED_DLL_TARGETS "")

# fsguilib_gl1, fsguilib_gl2 must be added after fsguilib due to gcc's link-order restriction, which I think is very dumb.
if(MSVC)
	target_link_libraries(ysflight${BITNESS}_gl1demo 
		ysgl
		yssystemfont
		fsguilib_gl1
		ysflight_platform_win32 
		ysflight_platform_win32_gl 
		ysflight_graphics_gl1 
		ysscenery_dnm_gl1
		ystexturemanager_gl
		geblgl_gl1
		ysglcpp_gl1
		fslazywindow
		${OS_LIBRARIES}
	)
	target_link_libraries(ysflight${BITNESS}_gl2demo 
		ysgl
		yssystemfont
		fsguilib_gl2 
		ysflight_platform_win32 
		ysflight_platform_win32_gl 
		ysflight_graphics_gl2
		ysscenery_dnm_gl2
		ystexturemanager_gl
		geblgl_gl2
		ysglcpp
		ysglcpp_gl2
		fslazywindow
		${OS_LIBRARIES}
	)

	set(DLL_PROJS sndYsflight${BITNESS} voiceYsflight${BITNESS})
	set(NEED_DLL_TARGETS ysflight${BITNESS}_gl1demo ysflight${BITNESS}_gl2demo)
elseif(APPLE)
	target_link_libraries(ysflight${BITNESS}_gl1demo 
		ysgl
		yssystemfont
		fsguilib_gl1
		ysflight_platform_macosx
		ysflight_graphics_gl1
		ysscenery_dnm_gl1
		ystexturemanager_gl
		${OS_LIBRARIES}
		ysjoystick
		ysjoycalibdlg_gl1
		geblgl_gl1
		ysglcpp_gl1
		fslazywindow
	)
	set_target_properties(ysflight${BITNESS}_gl1demo PROPERTIES MACOSX_BUNDLE_BUNDLE_NAME ysflight)
	set_target_properties(ysflight${BITNESS}_gl1demo PROPERTIES MACOSX_BUNDLE_ICON_FILE APPL.icns)
	set_target_properties(ysflight${BITNESS}_gl1demo PROPERTIES MACOSX_BUNDLE_INFO_STRING "YS Flight Simulator")
	set_target_properties(ysflight${BITNESS}_gl1demo PROPERTIES MACOSX_BUNDLE_GUI_IDENTIFIER com.YSFLIGHT.YsFlightSimulator)
	set_target_properties(ysflight${BITNESS}_gl1demo PROPERTIES MACOSX_BUNDLE_LONG_VERSION_STRING 20141222)
	set_target_properties(ysflight${BITNESS}_gl1demo PROPERTIES MACOSX_BUNDLE_SHORT_VERSION_STRING 20141222)
	set_target_properties(ysflight${BITNESS}_gl1demo PROPERTIES MACOSX_BUNDLE_BUNDLE_VERSION 20141222)
	set_target_properties(ysflight${BITNESS}_gl1demo PROPERTIES MACOSX_BUNDLE_COPYRIGHT "Copyright CaptainYS")

	target_link_libraries(ysflight${BITNESS}_gl2demo 
		ysgl
		yssystemfont
		fsguilib_gl2
		ysflight_platform_macosx
		ysflight_graphics_gl2
		ysscenery_dnm_gl2
		ystexturemanager_gl
		${OS_LIBRARIES}
		ysjoystick
		ysjoycalibdlg_gl2
		geblgl_gl2
		ysglcpp_gl2
		fslazywindow
	)
	set_target_properties(ysflight${BITNESS}_gl2demo PROPERTIES MACOSX_BUNDLE_BUNDLE_NAME ysflight)
	set_target_properties(ysflight${BITNESS}_gl2demo PROPERTIES MACOSX_BUNDLE_ICON_FILE APPL.icns)
	set_target_properties(ysflight${BITNESS}_gl2demo PROPERTIES MACOSX_BUNDLE_INFO_STRING "YS Flight Simulator")
	set_target_properties(ysflight${BITNESS}_gl2demo PROPERTIES MACOSX_BUNDLE_GUI_IDENTIFIER com.YSFLIGHT.YsFlightSimulator)
	set_target_properties(ysflight${BITNESS}_gl2demo PROPERTIES MACOSX_BUNDLE_LONG_VERSION_STRING 20141222)
	set_target_properties(ysflight${BITNESS}_gl2demo PROPERTIES MACOSX_BUNDLE_SHORT_VERSION_STRING 20141222)
	set_target_properties(ysflight${BITNESS}_gl2demo PROPERTIES MACOSX_BUNDLE_BUNDLE_VERSION 20141222)
	set_target_properties(ysflight${BITNESS}_gl2demo PROPERTIES MACOSX_BUNDLE_COPYRIGHT "Copyright CaptainYS")

	set(DLL_PROJS sndYsflight${BITNESS} voiceYsflight${BITNESS})
	set(NEED_DLL_TARGETS ysflight${BITNESS}_gl1demo ysflight${BITNESS}_gl2demo)
elseif(ANDROID)
	target_link_libraries(ysflight${BITNESS}_gl2demo 
		ysgl
		yssystemfont
		fsguilib_gl2 
		ysflight_platform_android 
		ysscenery_dnm_gl2
		ystexturemanager_gl
		${OS_LIBRARIES}
		ysjoystick
		ysjoycalibdlg_gl2
		ysflight_graphics_gl2
		fslazywindow
	)
elseif(UNIX)
	target_link_libraries(ysflight${BITNESS}_gl1demo 
		ysgl
		yssystemfont
		fsguilib_gl1
		ysflight_platform_linux 
		ysscenery_dnm_gl1
		ystexturemanager_gl
		${OS_LIBRARIES}
		ysjoystick
		ysjoycalibdlg_gl1
		ysflight_graphics_gl1 
		fslazywindow
	)
	target_link_libraries(ysflight${BITNESS}_gl2demo 
		ysgl
		yssystemfont
		fsguilib_gl2 
		ysflight_platform_linux 
		ysscenery_dnm_gl2
		ystexturemanager_gl
		${OS_LIBRARIES}
		ysjoystick
		ysjoycalibdlg_gl2
		ysflight_graphics_gl2
		fslazywindow
	)

	set(DLL_PROJS sndYsflight${BITNESS} sndYsflight${BITNESS}-pulse voiceYsflight${BITNESS})
	set(NEED_DLL_TARGETS ysflight${BITNESS}_gl1demo ysflight${BITNESS}_gl2demo)
endif()



foreach(ONE_TARGET ${NEED_DLL_TARGETS})
	get_property(DATA_DESTINATION TARGET ${ONE_TARGET} PROPERTY YS_DATA_COPY_DIR)
	add_dependencies(${ONE_TARGET} ${DLL_PROJS})
	foreach(DLL_TARGET ${DLL_PROJS})
		add_custom_command(TARGET ${ONE_TARGET} POST_BUILD 
			COMMAND echo For ${ONE_TARGET}
			COMMAND echo DLL $<TARGET_FILE:${DLL_TARGET}>
			COMMAND "${CMAKE_COMMAND}" -E copy \"$<TARGET_FILE:${DLL_TARGET}>\" \"${DATA_DESTINATION}/plugin/.\"
			)
	endforeach(DLL_TARGET)
endforeach(ONE_TARGET)



foreach(ONE_TARGET ${TARGET_NAME})
	add_custom_command(TARGET ${ONE_TARGET} PRE_BUILD 
		COMMAND "python" "${CMAKE_CURRENT_SOURCE_DIR}/../../scripts/runtime_timestamp.py" "${CMAKE_CURRENT_SOURCE_DIR}/../../runtime")
endforeach(ONE_TARGET)
