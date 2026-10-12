cmake_minimum_required(VERSION 3.20)
project(CGOL+ LANGUAGES CXX)

# version
set(PROJECT_VERSION "0.1.0")
set(PROJECT_VERSION_MAJOR 0)
set(PROJECT_VERSION_MINOR 1)
set(PROJECT_VERSION_PATCH 0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set_property(DIRECTORY PROPERTY DESCRIPTION "CGOL+")
set_property(DIRECTORY PROPERTY HOMEPAGE_URL "https://github.com/ZaineRehman/CGOL-plus")


# set output directory for build objects
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin/$<CONFIGURATION>")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin/$<CONFIGURATION>")

# sources
add_executable(CGOLplus 
	${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
)


# SDL3 path
set(SDL3_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/SDL3)

# headers
target_include_directories(CGOLplus PRIVATE 
	${CMAKE_CURRENT_SOURCE_DIR}/include
	${SDL3_DIR}/include
)

# target lib for linking
target_link_directories(CGOLplus PRIVATE ${SDL3_DIR}/lib)
target_link_libraries(CGOLplus PRIVATE SDL3)

# copy SDL3 dll to build output dir
add_custom_command(TARGET CGOLplus POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_if_different
	${SDL3_DIR}/bin/SDL3.dll
	$<TARGET_FILE_DIR:CGOLplus>
	COMMENT "Copying SDL3.dll to target output directory..."
)


# packaging

set(CPACK_PACKAGE_NAME "CGOL+")
set(CPACK_PACKAGE_VENDOR "Zaine Rehman")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "CGOL+")

if(WIN32)
	set(CPACK_GENERATOR ZIP)
else()
	set(CPACK_GENERATOR TGZ)
endif()

include(CPack)