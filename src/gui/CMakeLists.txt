set(TARGET_NAME ysflight_gui)
set(LIB_DEPENDENCY ysflight_ui ysjoystick)

set(CURL_DIR "")


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

find_package(CURL CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

add_library(${TARGET_NAME} ${SRCS} ${HEADERS})
target_link_libraries(${TARGET_NAME} ${LIB_DEPENDENCY})
target_link_libraries(${TARGET_NAME} ${LIB_DEPENDENCY} CURL::libcurl)
target_link_libraries(${TARGET_NAME} ${LIB_DEPENDENCY} nlohmann_json::nlohmann_json)
target_include_directories(${TARGET_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
