set(TARGET_NAME ysflight_graphics_gl2)
set(LIB_DEPENDENCY ysglcpp ysglcpp_gl2 geblkernel geblgl geblgl_gl2 ysflight_common ysflight_graphics_common ysflight_core ysclass fssimplewindow ysbitmap fsguilib ysbitmapfont ysfontrenderer yssystemfont ystexturemanager_gl ystexturemanager ysscenery_dnm ysjoystick ysjoycalibdlg_gl2)

set(SRCS
fsblackoutgl2.0.cpp
fscloudgl2.0.cpp
fsexistencegl2.0.cpp
fsexplosiongl2.0.cpp
fsfontrenderergl2.0.cpp
fsgl2bitmap.cpp
fsgroundpropertygl2.0.cpp
fsgroundskygl2.0.cpp
fshud2gl2.0.cpp
fshudgl2.0.cpp
fsinstpanelgl2.0.cpp
fsopengl2.0.cpp
fsparticlegl2.0.cpp
fsweapongl2.0.cpp
fsweathergl2.0.cpp
)

set(HEADERS
fsgl2.0util.h
fsgl2bitmap.h
fsopengl2.0.h
)

add_library(${TARGET_NAME} ${SRCS} ${HEADERS})
target_link_libraries(${TARGET_NAME} ${LIB_DEPENDENCY})
target_include_directories(${TARGET_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
