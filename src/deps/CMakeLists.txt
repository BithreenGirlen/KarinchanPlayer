cmake_minimum_required (VERSION 3.25)
project ("KarinchanPlayer-deps")

set(SPINE_VERSION "4.1")
set(SPINE_RUNTIME_LANGUAGE "spine-cpp")

set(SPINE_RUNTIME_DIR ${CMAKE_CURRENT_LIST_DIR}/${SPINE_RUNTIME_LANGUAGE}-${SPINE_VERSION})

set(DXLIB_DIR ${CMAKE_CURRENT_LIST_DIR}/dxlib)

set(TEMP_DIR ${CMAKE_CURRENT_BINARY_DIR}/temp)

function(download_and_extract HOST_URL ARCHIVE_NAME)
	if (NOT EXISTS "${TEMP_DIR}/${ARCHIVE_NAME}.zip")
		file(DOWNLOAD "${HOST_URL}" "${TEMP_DIR}/${ARCHIVE_NAME}.zip" SHOW_PROGRESS)
	endif()

	if (NOT EXISTS "${TEMP_DIR}/${ARCHIVE_NAME}")
		file(ARCHIVE_EXTRACT INPUT "${TEMP_DIR}/${ARCHIVE_NAME}.zip" DESTINATION "${TEMP_DIR}")
	endif()
endfunction()

# Download DxLib for VC.
if (NOT EXISTS "${DXLIB_DIR}")
	set (DXLIB_VC_URL "https://dxlib.xsrv.jp/temp/DxLibVCTest.zip")
	set(DXLIB_VC_FOLDER_NAME "DxLibVCTest")
	download_and_extract("${DXLIB_VC_URL}" "${DXLIB_VC_FOLDER_NAME}")

	file(GLOB DXLIB_LIBS "${TEMP_DIR}/${DXLIB_VC_FOLDER_NAME}/*.lib")
	foreach(FILE IN LISTS DXLIB_LIBS)
		if(FILE MATCHES "_vs2015_x64_MDd.lib" OR FILE MATCHES "_vs2015_x64_MD.lib")
			file(COPY ${FILE} DESTINATION "${DXLIB_DIR}")
		endif()
	endforeach()

	file(GLOB INCLUDE_FILES "${TEMP_DIR}/${DXLIB_VC_FOLDER_NAME}/*.h")
	foreach(FILE IN LISTS INCLUDE_FILES)
		file(COPY ${FILE} DESTINATION "${DXLIB_DIR}")
	endforeach()
endif()

# Download Spine runtime
if (NOT EXISTS "${SPINE_RUNTIME_DIR}/lib")
	set (SPINE_RUNTIME_URL "https://github.com/EsotericSoftware/spine-runtimes/archive/refs/heads/${SPINE_VERSION}.zip")
	set (SPINE_FOLDER_NAME "spine-runtimes-${SPINE_VERSION}")
	download_and_extract("${SPINE_RUNTIME_URL}" "${SPINE_FOLDER_NAME}")

	file(COPY "${TEMP_DIR}/${SPINE_FOLDER_NAME}/${SPINE_RUNTIME_LANGUAGE}/${SPINE_RUNTIME_LANGUAGE}/src" DESTINATION "${SPINE_RUNTIME_DIR}")
	file(COPY "${TEMP_DIR}/${SPINE_FOLDER_NAME}/${SPINE_RUNTIME_LANGUAGE}/${SPINE_RUNTIME_LANGUAGE}/include" DESTINATION "${SPINE_RUNTIME_DIR}")
endif()

if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:Embedded>,$<$<CONFIG:Debug,RelWithDebInfo>:Embedded>>")
endif()

# Build Spine runtime as static library
add_subdirectory ("spine-cpp-4.1")
