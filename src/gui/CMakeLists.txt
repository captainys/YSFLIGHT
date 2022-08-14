set(TARGET_NAME ysflight_gui)
set(LIB_DEPENDENCY ysflight_ui ysjoystick)

set(SRCS
fsmenu.cpp
fsmenu_file.cpp
fsmenu_net.cpp
fsmenu_sim.cpp
fsmenu_util.cpp
fsmenu_option.cpp
fsguicommondialog.cpp
fsguinetdialog.cpp
fsguimaincanvas.cpp
fsguisiminfodialog.cpp
)

set(HEADERS
fsmenu.h
fsguicommondialog.h
fsguinetdialog.h
fsguimaincanvas.h
fsguisiminfodialog.h
)

add_library(${TARGET_NAME} ${SRCS} ${HEADERS})
target_link_libraries(${TARGET_NAME} ${LIB_DEPENDENCY})
target_include_directories(${TARGET_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
