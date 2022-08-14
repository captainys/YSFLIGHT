set(TARGET_NAME ysflight_config)
set(LIB_DEPENDENCY ysclass ysport ysflight_util ysflight_common)
set(SRCS
fsconfig.cpp
fsoption.cpp
fsnetconfig.cpp
)
set(HEADERS
fsconfig.h
fsoption.h
fsnetconfig.h
)

add_library(${TARGET_NAME} ${SRCS} ${HEADERS})
target_link_libraries(${TARGET_NAME} ${LIB_DEPENDENCY})
target_include_directories(${TARGET_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
