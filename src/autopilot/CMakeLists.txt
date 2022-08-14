set(TARGET_NAME ysflight_autopilot)
set(LIB_DEPENDENCY 
	geblkernel 
	geblgl 
	ysclass 
	ysflight_core
	ysflight_common 
	yssocket 
	ysflight_platform_common 
	ysflight_graphics_common 
	ysscenery_dnm 
	ysport 
	ysflight_dynamics 
	ysflight_filename
	fssimplewindow
)
set(SRCS
fsairshowacro.cpp
fsautopilot.cpp
fsautopilotexperimental.cpp
fsdefenderautopilot.cpp
fsdogfightautopilot.cpp
fsformationautopilot.cpp
fsgroundattackautopilot.cpp
fsmissionautopilot.cpp
fsrefuelandtakeoff.cpp
)
set(HEADERS
fsautopilot.h
fsautopilotexperimental.h
fsdefenderautopilot.h
fsdogfightautopilot.h
fsformationautopilot.h
fsgroundattackautopilot.h
fsmissionautopilot.h
fsrefuelandtakeoff.h
)

add_library(${TARGET_NAME} ${SRCS} ${HEADERS})
target_link_libraries(${TARGET_NAME} ${LIB_DEPENDENCY})
target_include_directories(${TARGET_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
