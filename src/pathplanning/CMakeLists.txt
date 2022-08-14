set(TARGET_NAME "ysflight_pathplanning")
set(LIB_DEPENDENCY ysclass)

set(SRCS
fsthreatavoidingpath.cpp
)

set(HEADERS
)

add_library(${TARGET_NAME} ${SRCS} ${HEADERS})
target_link_libraries(${TARGET_NAME} ${LIB_DEPENDENCY})
target_include_directories(${TARGET_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
