if(CMAKE_SIZEOF_VOID_P EQUAL 8)
	set(BITNESS 64)
else()
	set(BITNESS 32)
endif()

set(TARGET_NAME ysflight_common)
set(LIB_DEPENDENCY
geblkernel
geblgl
fsguilib
fsguifiledialog
ysbitmap
ysbitmapfont
ysfontrenderer
ysport
ystexturemanager
ysclass
ysclass11
ysglcpp
)
set(SRCS
fsdef.cpp
fstextresource.cpp
)

set(HEADERS
fsdef.h
fsairsound.h
fstextresource.h
fsrecord.h
)

add_library(${TARGET_NAME} ${SRCS} ${HEADERS})
target_link_libraries(${TARGET_NAME} ${LIB_DEPENDENCY})
target_include_directories(${TARGET_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
