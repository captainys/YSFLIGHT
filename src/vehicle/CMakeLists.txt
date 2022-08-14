set(TARGET_NAME "ysflight_vehicle")
set(LIB_DEPENDENCY 
ysscenery_dnm 
ysflight_core
ysflight_common 
ysflight_platform_common 
ysflight_graphics_common 
ysflight_dynamics 
ysflight_util 
yssocket
geblkernel 
geblgl 
ysclass 
ysport 
)

# Don't link fssimplewindow.  It can be fssimplenowindow.  Just add include directory
get_property(ADDITIONAL_INCLUDE_DIR TARGET fssimplewindow PROPERTY INCLUDE_DIRECTORIES)

set(SRCS
fsairplaneproperty.cpp
fsgroundproperty.cpp
fsproperty.cpp
fsvehicleproperty.cpp
fsnavaid.cpp
)

set(HEADERS
fsairplaneproperty.h
fsgroundproperty.h
fsproperty.h
fsvehicleproperty.h
fsnavaid.h
)


add_library(${TARGET_NAME} ${SRCS} ${HEADERS})
target_link_libraries(${TARGET_NAME} ${LIB_DEPENDENCY})
target_include_directories(${TARGET_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${ADDITIONAL_INCLUDE_DIR})
