set(TARGET_NAME "ysflight_util")
set(LIB_DEPENDENCY ysclass ysport ysflight_common)

set(SRCS
fsbase.cpp
fsutil.cpp
)

set(HEADERS
fsnetutil.h
)

add_library(${TARGET_NAME} ${SRCS} ${HEADERS})
target_link_libraries(${TARGET_NAME} ${LIB_DEPENDENCY})
target_include_directories(${TARGET_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
