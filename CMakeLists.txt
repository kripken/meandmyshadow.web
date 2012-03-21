PROJECT(meandmyshadow)
CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

Find_Package(SDL REQUIRED)
Find_Package(SDL_ttf REQUIRED)
Find_Package(SDL_image REQUIRED)
Find_Package(SDL_mixer REQUIRED)
Find_Package(SDL_gfx REQUIRED)

configure_file (
  "${PROJECT_SOURCE_DIR}/src/config.h.in"
  "${PROJECT_BINARY_DIR}/config.h"
  )

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR})
set(SRC_DIR ${PROJECT_SOURCE_DIR}/src)

include_directories(
	${PROJECT_BINARY_DIR}
)

SET(CMAKE_CXX_FLAGS "-Wall")

# Déclaration de l'exécutable
file(GLOB Sources ${SRC_DIR}/*.cpp)
add_executable(meandmyshadow ${Sources})

target_link_libraries(
    meandmyshadow
)

install(DIRECTORY ${PROJECT_SOURCE_DIR}/data DESTINATION share/meandmyshadow/)
install(TARGETS meandmyshadow RUNTIME DESTINATION bin)
if("${CMAKE_SYSTEM_NAME}" MATCHES "Linux") 
	install(FILES meandmyshadow.desktop DESTINATION share/applications/)
	install(FILES icons/16x16/meandmyshadow.png DESTINATION share/icons/hicolor/16x16/apps/)
	install(FILES icons/32x32/meandmyshadow.png DESTINATION share/icons/hicolor/32x32/apps/)
	install(FILES icons/48x48/meandmyshadow.png DESTINATION share/icons/hicolor/48x48/apps/)
	install(FILES icons/64x64/meandmyshadow.png DESTINATION share/icons/hicolor/64x64/apps/)
endif("${CMAKE_SYSTEM_NAME}" MATCHES "Linux")
