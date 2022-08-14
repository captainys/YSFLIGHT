if("${CMAKE_SYSTEM_NAME}" STREQUAL "WindowsStore")
	return()
endif()


set(TARGET_NAME ysflight_graphics_null)
set(LIB_DEPENDENCY ysflight_common ysflight_graphics_common ysflight_core geblkernel geblgl geblgl_nownd ysglcpp ysglcpp_nownd ysclass ysbitmap fsguilib ysbitmapfont ysfontrenderer ysnullsystemfont ystexturemanager_nownd ystexturemanager ysscenery_dnm yssystemfont)

set(SRCS
fsblackoutnownd.cpp
fscloudnownd.cpp
fsexistencenownd.cpp
fsexplosionnownd.cpp
fsfontrenderernownd.cpp
fsgroundpropertynownd.cpp
fsgroundskynownd.cpp
fshud2nownd.cpp
fshudnownd.cpp
fsinstpanelnownd.cpp
fsnownd.cpp
fsparticlenownd.cpp
fsweaponnownd.cpp
fsweathernownd.cpp
)

set(HEADERS
)

add_library(${TARGET_NAME} ${SRCS} ${HEADERS})
target_link_libraries(${TARGET_NAME} ${LIB_DEPENDENCY})
target_include_directories(${TARGET_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
