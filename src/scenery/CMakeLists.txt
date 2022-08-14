set(TARGET_NAME ysscenery_dnm ysscenery_dnm_gl1 ysscenery_dnm_gl2 ysscenery_dnm_nownd)
set(LIB_DEPENDENCY ysclass ystexturemanager ysflight_util ysflight_common geblgl ysgl)

set(SRCS
ysscenery.cpp
yssceneryio.cpp
sescenery.cpp
)

set(HEADERS
ysgltess.h
ysscenery.h
sescenery.h
)

add_library(ysscenery_dnm ${SRCS})
target_include_directories(ysscenery_dnm PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(ysscenery_dnm ${LIB_DEPENDENCY})

if(ysd3d9_EXIST)
	add_library(ysscenery_dnm_d3d yssceneryd3d.cpp)
	target_include_directories(ysscenery_dnm_d3d PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
	target_link_libraries(ysscenery_dnm_d3d ysd3d9 ystexturemanager_d3d9 ${LIB_DEPENDENCY})
endif()

add_library(ysscenery_dnm_gl1 ysscenerygl.cpp ysgltess.cpp)
target_include_directories(ysscenery_dnm_gl1 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(ysscenery_dnm_gl1 ystexturemanager_gl ${LIB_DEPENDENCY})

add_library(ysscenery_dnm_gl2 ysscenerygl2.0.cpp)
target_include_directories(ysscenery_dnm_gl2 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(ysscenery_dnm_gl2 ystexturemanager_gl ${LIB_DEPENDENCY})

add_library(ysscenery_dnm_nownd ysscenerynownd.cpp)
target_include_directories(ysscenery_dnm_nownd PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(ysscenery_dnm_nownd ystexturemanager_nownd ${LIB_DEPENDENCY})

include_directories(${CMAKE_SOURCE_DIR}/trunk/src/ysflight/src)
