set(TARGET_NAME ysflight_graphics_common)
set(LIB_DEPENDENCY ysflight_common ysflight_filename fsguilib geblkernel geblgl ystexturemanager)

# Don't link fssimplewindow.  It can be fssimplenowindow.  Just add include directory
get_property(ADDITIONAL_INCLUDE_DIR TARGET fssimplewindow PROPERTY INCLUDE_DIRECTORIES)

set(SRCS
fswirefont.cpp
fsvisual.cpp
fsblackout.cpp
fsfontrenderer.cpp
fsconsole.cpp
fsopengl.cpp
fstexturemanager.cpp
fswirefont_makebuf.cpp
)

set(HEADERS
fsopengl.h
fswirefont.h
fsvisual.h
fsblackout.h
fsfontrenderer.h
fsconsole.h
fstexturemanager.h
)

add_library(${TARGET_NAME} ${SRCS} ${HEADERS})
target_link_libraries(${TARGET_NAME} ${LIB_DEPENDENCY})
target_include_directories(${TARGET_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${ADDITIONAL_INCLUDE_DIR})
