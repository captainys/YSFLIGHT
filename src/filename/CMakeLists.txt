set(TARGET_NAME ysflight_filename)
set(LIB_DEPENDENCY ysclass ysport ysflight_common)

set(SRCS
fsfilename.cpp
)

set(HEADERS
fsfilename.h
)

add_library(${TARGET_NAME} ${SRCS} ${HEADERS})
target_link_libraries(${TARGET_NAME} ${LIB_DEPENDENCY})
target_include_directories(${TARGET_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
