if("${CMAKE_SYSTEM_NAME}" STREQUAL "WindowsStore")
	return()
endif()


set(TARGET_NAME ysflight_graphics_gl1)
set(LIB_DEPENDENCY ysglcpp ysglcpp_gl1 geblkernel geblgl geblgl_gl1 ysflight_common ysflight_core ysflight_graphics_common ysclass fssimplewindow ysbitmap fsguilib ysbitmapfont ysfontrenderer yssystemfont ystexturemanager_gl ystexturemanager ysscenery_dnm ysjoystick ysjoycalibdlg_gl1)

set(SRCS
fsblackoutgl.cpp
fscloudgl.cpp
fsexistencegl.cpp
fsexplosiongl.cpp
fsfontrenderergl.cpp
fsgroundpropertygl.cpp
fsgroundskygl.cpp
fshud2gl.cpp
fshudgl.cpp
fsinstpanelgl.cpp
fsopengl.cpp
fsparticlegl.cpp
fsweapongl.cpp
fsweathergl.cpp
)

set(HEADERS
)

add_library(${TARGET_NAME} ${SRCS} ${HEADERS})
target_link_libraries(${TARGET_NAME} ${LIB_DEPENDENCY})
target_include_directories(${TARGET_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
