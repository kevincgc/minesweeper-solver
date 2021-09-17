# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.8)

project ("Minesweeper")

find_program(XMLLINT NAMES xmllint REQUIRED)
find_program(GLIB_COMPILE_RESOURCES NAMES glib-compile-resources REQUIRED)

set(GRESOURCE_C m.gresources.c)
set(GRESOURCE_XML m.gresources.xml)
set(GRESOURCE_DEPENDENCIES)

execute_process(
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/gresources
	COMMAND ${GLIB_COMPILE_RESOURCES} --generate-dependencies ${GRESOURCE_XML}
	OUTPUT_VARIABLE GRESOURCE_DEPENDENCIES
)
string(REPLACE "\n" ";${CMAKE_CURRENT_SOURCE_DIR}/gresources/" GRESOURCE_DEPENDENCIES ${GRESOURCE_DEPENDENCIES})
list(POP_BACK GRESOURCE_DEPENDENCIES)

add_custom_command(
	OUTPUT ${GRESOURCE_C}
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/gresources
	COMMAND set XMLLINT=${XMLLINT}
	COMMAND ${GLIB_COMPILE_RESOURCES}
	ARGS
		--target=${CMAKE_CURRENT_BINARY_DIR}/${GRESOURCE_C}
		--generate-source
		${GRESOURCE_XML}
	VERBATIM
	MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/gresources/${GRESOURCE_XML}
	DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/gresources/${GRESOURCE_DEPENDENCIES}
)

add_custom_target(
	m-resource
	DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${GRESOURCE_C}
)

# Add source to this project's executable.
add_executable (Minesweeper "src/Minesweeper.cpp" "src/game_application.cpp" "src/game_window.cpp" "src/main.cpp" ${CMAKE_CURRENT_BINARY_DIR}/${GRESOURCE_C})

SET(MS_LINKFLAGS ${CMAKE_CURRENT_SOURCE_DIR}/Minesweeper.res)
SET_TARGET_PROPERTIES(Minesweeper PROPERTIES LINK_FLAGS ${MS_LINKFLAGS})

if(NOT ${CMAKE_BUILD_TYPE} MATCHES "Debug")
	target_link_options(Minesweeper PRIVATE "/SUBSYSTEM:WINDOWS" "/ENTRY:mainCRTStartup")
endif()

set_source_files_properties(
	${CMAKE_CURRENT_BINARY_DIR}/${GRESOURCE_C}
	PROPERTIES zD TRUE
)

add_dependencies(${PROJECT_NAME} m-resource)

set(PKG_CONFIG_EXECUTABLE "C:/src/vcpkg/installed/x86-windows/tools/pkgconf/pkgconf.exe")
find_package(PkgConfig)
pkg_check_modules(GTKMM_VARS REQUIRED IMPORTED_TARGET gtkmm-4.0)

target_link_libraries(Minesweeper PRIVATE PkgConfig::GTKMM_VARS)