set(TARGET_NAME "ysflight_dynamics")
set(LIB_DEPENDENCY ysclass ysport ysflight_common ysflight_util)

set(SRCS
fsengine.cpp
fsrealprop.cpp
fsairproperty.cpp
)

set(HEADERS
fsengine.h
fsrealprop.h
fsairproperty.h
)
add_library(${TARGET_NAME} ${SRCS} ${HEADERS})
target_link_libraries(${TARGET_NAME} ${LIB_DEPENDENCY})
target_include_directories(${TARGET_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
