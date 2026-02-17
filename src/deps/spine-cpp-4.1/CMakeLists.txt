cmake_minimum_required (VERSION 3.25)

set(SPINE_LIBRARY_NAME "spine-cpp")

file(GLOB INCLUDES "include/**/*.h")
file(GLOB SOURCES "src/**/*.cpp")

add_library("${SPINE_LIBRARY_NAME}" STATIC ${SOURCES} ${INCLUDES})
target_include_directories("${SPINE_LIBRARY_NAME}" PUBLIC include)

if(MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

# set_target_properties("${SPINE_LIBRARY_NAME}" PROPERTIES DEBUG_POSTFIX "-d")

install(
	TARGETS "${SPINE_LIBRARY_NAME}"
	ARCHIVE DESTINATION "${CMAKE_CURRENT_SOURCE_DIR}/lib/x64-${CMAKE_BUILD_TYPE}/"
)
