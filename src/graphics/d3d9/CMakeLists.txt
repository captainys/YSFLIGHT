if("${CMAKE_SYSTEM_NAME}" STREQUAL "WindowsStore")
	return()
endif()


set(TARGET_NAME ysflight_graphics_d3d9)
set(LIB_DEPENDENCY geblkernel geblgl ysflight_common ysflight_core ysglcpp_d3d9 ysflight_graphics_common ysclass fssimplewindow ysd3d9 ysbitmap fsguilib ysbitmapfont ysfontrenderer yssystemfont ystexturemanager_d3d9 ystexturemanager ysscenery_dnm)

set(SRCS
fsblackoutd3d.cpp
fscloudd3d.cpp
fsd3d.cpp
fsexistenced3d.cpp
fsexplosiond3d.cpp
fsfontrendererdx.cpp
fsgroundpropertyd3d.cpp
fsgroundskyd3d.cpp
fshud2d3d.cpp
fshudd3d.cpp
fsinstpaneld3d.cpp
fsparticled3d.cpp
fsweapond3d.cpp
fsweatherd3d.cpp
)

set(HEADERS
fsd3d.h
)

add_library(${TARGET_NAME} ${SRCS} ${HEADERS})
target_link_libraries(${TARGET_NAME} ${LIB_DEPENDENCY})
target_include_directories(${TARGET_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
