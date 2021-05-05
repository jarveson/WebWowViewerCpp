cmake_minimum_required(VERSION 3.18)

project(AWebWoWViewerCpp)
include(ExternalProject)

enable_language( C CXX ASM )

set(CMAKE_VERBOSE_MAKEFILE ON)

if(POLICY CMP0077)
	cmake_policy(SET CMP0077 NEW)
endif()
if(COMMAND cmake_policy)
	cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)
message("Test message")

set(USE_NEON 0)

include_directories(BEFORE SYSTEM "${NDK_PATH}/include/c++/4.9.x/" "${NDK_PATH}/sysroot/usr/include/")
message(ERROR " CMAKE_SYSTEM_PROCESSOR = ${CMAKE_SYSTEM_PROCESSOR}")
message(ERROR " ANDROID_ABI = ${ANDROID_ABI}")
#TODO: check ANDROID_ARM_NEON too
if(${ANDROID_ABI} STREQUAL "armeabi-v7a")
	include_directories(${ANDROID_SYSROOT}/usr/include/arm-linux-androideabi)
	set(USE_NEON 1)
elseif(${ANDROID_ABI} STREQUAL "arm64-v8a")
	include_directories(${ANDROID_SYSROOT}/usr/include/aarch64-linux-android)
	set(USE_NEON 1)
elseif(${ANDROID_ABI} STREQUAL "x86_64")
	include_directories(${ANDROID_SYSROOT}/usr/include/x86_64-linux-android)
	set(USE_NEON 0)
else()
	include_directories(${ANDROID_SYSROOT}/usr/include/arm-linux-androideabi)
	set(USE_NEON 0)
endif()



if (WIN32 AND MSVC)
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /DWIN64")
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /DWIN64")
        set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /DWIN64")
		set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /DWIN64")
    endif()
 endif()
message(CMAKE_BUILD_TYPE = ${CMAKE_BUILD_TYPE})
#set(CMAKE_BUILD_TYPE Release)
#set(CMAKE_C_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_GLIBCXX_DEBUG -D_GLIBCXX_DEBUG_PEDANTIC ")
#set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_GLIBCXX_DEBUG -D_GLIBCXX_DEBUG_PEDANTIC  ")

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
	message("This is clang")
	#Force clang to use libc++
	set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -static ")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -static -stdlib=libc++")
	set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -static ")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -static -stdlib=libc++")
#	link_libraries(libstdc++fs)


	#set(_GLIBCXX_USE_C99_CHECK 0)
	#set(_GLIBCXX_USE_C99_DYNAMIC 0)
	#set(_GLIBCXX_USE_C99_LONG_LONG_CHECK 0)
	#set(_GLIBCXX_USE_C99_LONG_LONG_DYNAMIC 0)
#	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -stdlib=libc++")
#	set(CMAKE_C_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_DEBUG} -stdlib=libc++")
#	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -stdlib=libc++")
endif()
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	link_libraries(stdc++fs)
#	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -pthread")
#	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -pthread ")
#	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20")
endif()


#set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_GLIBCXX_DEBUG -pthread")

#ZLib library
set(BUILD_SHARED_LIBS OFF)
set(VIEWER_IMPORT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../..)
add_subdirectory(${VIEWER_IMPORT_DIR}/3rdparty/zlib ${CMAKE_CURRENT_BINARY_DIR}/zlib)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/3rdparty/zlib)
set(BUILD_SHARED_LIBS OFF)
if (MSVC)
	set(ZLIB_INCLUDE_DIR ${VIEWER_IMPORT_DIR}/3rdparty/zlib ${CMAKE_BINARY_DIR}/3rdparty/zlib)
	message("CMAKE_BINARY_DIR = ${CMAKE_BINARY_DIR}")
	message("CMAKE_BUILD_TYPE = ${CMAKE_BUILD_TYPE}")
	if (CMAKE_BUILD_TYPE STREQUAL "Debug")
		set(ZLIB_LIBRARY "${CMAKE_BINARY_DIR}/3rdparty/zlib/zlibd.lib")
	else()
		set(ZLIB_LIBRARY "${CMAKE_BINARY_DIR}/3rdparty/zlib/zlib.lib")
	endif()
	set(ZLIB_INCLUDE_DIRS ${VIEWER_IMPORT_DIR}/3rdparty/zlib)
endif()


#libpng
if (MSVC)
	#set(PNG_BUILD_ZLIB ON)
	set(PNG_BUILD_ZLIB OFF)
else()
	set(PNG_BUILD_ZLIB OFF)
endif()

set(PNG_ARM_NEON off CACHE STRING "Shut off .s files for clang compiler" FORCE)

add_subdirectory(${VIEWER_IMPORT_DIR}/3rdparty/libpng ${CMAKE_CURRENT_BINARY_DIR}/libpng)
include_directories(${VIEWER_IMPORT_DIR}/3rdparty/libpng)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/libpng)
include_directories(${VIEWER_IMPORT_DIR}/3rdparty/SQLiteCpp/sqlite3)

#c++ 17 FS implementation
add_subdirectory(${VIEWER_IMPORT_DIR}/3rdparty/filesystem_impl ${CMAKE_CURRENT_BINARY_DIR}/filesystem_impl)

#bz2renderFlag
set(BUILD_SHARED_LIBS OFF)
add_subdirectory(${VIEWER_IMPORT_DIR}/3rdparty/bzip ${CMAKE_CURRENT_BINARY_DIR}/bzip)

#mdebtls as replacement for openssl
set(USE_STATIC_MBEDTLS_LIBRARY ON CACHE BOOL "Use static" )
set(USE_SHARED_MBEDTLS_LIBRARY ON CACHE BOOL "Do not use shared" )
add_subdirectory(${VIEWER_IMPORT_DIR}/3rdparty/mdebtls ${CMAKE_CURRENT_BINARY_DIR}/mdebtls)
SET(MBEDCRYPTO_LIBRARY "mbedcrypto.so")
SET(MBEDTLS_LIBRARY "mbedx509.so")
SET(MBEDX509_LIBRARY "mbedtls.so")
set(MBEDTLS_INCLUDE_DIRS ${CMAKE_CURRENT_BINARY_DIR}/mdebtls/include ${CMAKE_CURRENT_SOURCE_DIR}/src/includes)

#cpr
set(CMAKE_USE_OPENSSL OFF CACHE BOOL "Use OpenSSL code. OFF" )
set(CMAKE_USE_MBEDTLS ON CACHE BOOL "ON" )
include(FetchContent)
FetchContent_Declare(cpr
		GIT_REPOSITORY https://github.com/whoshuu/cpr.git
		GIT_TAG c8d33915dbd88ad6c92b258869b03aba06587ff9 # the commit hash for 1.5.0
		)
#FetchContent_MakeAvailable(cpr)
FetchContent_GetProperties(cpr)
if(NOT cpr_POPULATED)
	MESSAGE("CPR IS NOT POPULATED")
	FetchContent_Populate(cpr)

	add_subdirectory(${cpr_SOURCE_DIR} ${cpr_BINARY_DIR})
else()
	MESSAGE("CPR IS POPULATED")
endif()


#CascLib
add_definitions(-DCASCLIB_NO_AUTO_LINK_LIBRARY)

if (CMAKE_NDK_BUILD MATCHES 1)
	#set(CASC_BUILD_SHARED_LIB ON)
	set(CMAKE_SYSTEM_NAME_TEMPTEMP ${CMAKE_SYSTEM_NAME})
	set(CMAKE_SYSTEM_NAME "Linux")
endif()
if (WIN32)
	set(WITH_LIBTOMCRYPT true)
endif()

option(CASC_BUILD_SHARED_LIB "" OFF)
set(CASC_BUILD_STATIC_LIB ON CACHE BOOL "Set static lib from main project")
add_subdirectory(${VIEWER_IMPORT_DIR}/3rdparty/casclib ${CMAKE_CURRENT_BINARY_DIR}/casclib)

if (CMAKE_NDK_BUILD MATCHES 1)
	target_compile_definitions(casc PUBLIC -DCMAKE_SYSTEM_NAME=Linux)
	set(CMAKE_SYSTEM_NAME "${CMAKE_SYSTEM_NAME_TEMPTEMP}")
endif()

add_subdirectory(${VIEWER_IMPORT_DIR}/wowViewerLib ${CMAKE_CURRENT_BINARY_DIR}/wowViewerLib)



set(SOURCE_FILES
        ${VIEWER_IMPORT_DIR}/src/persistance/httpFile/httpFile.cpp
        ${VIEWER_IMPORT_DIR}/src/persistance/httpFile/httpFile.h
        ${VIEWER_IMPORT_DIR}/src/persistance/RequestProcessor.cpp
        ${VIEWER_IMPORT_DIR}/src/persistance/RequestProcessor.h
        ${VIEWER_IMPORT_DIR}/src/persistance/ZipRequestProcessor.cpp
        ${VIEWER_IMPORT_DIR}/src/persistance/ZipRequestProcessor.h
#		src/persistance/HttpZipRequestProcessor.cpp
#		src/persistance/HttpZipRequestProcessor.h
        ${VIEWER_IMPORT_DIR}/src/persistance/CascRequestProcessor.cpp
        ${VIEWER_IMPORT_DIR}/src/persistance/CascRequestProcessor.h
        ${VIEWER_IMPORT_DIR}/src/persistance/HttpRequestProcessor.cpp
        ${VIEWER_IMPORT_DIR}/src/persistance/HttpRequestProcessor.h

        ${VIEWER_IMPORT_DIR}/src/ui/imguiLib/imgui.cpp
        ${VIEWER_IMPORT_DIR}/src/ui/imguiLib/imgui_demo.cpp
        ${VIEWER_IMPORT_DIR}/src/ui/imguiLib/imgui_draw.cpp
        ${VIEWER_IMPORT_DIR}/src/ui/imguiLib/imgui_widgets.cpp
        ${VIEWER_IMPORT_DIR}/src/ui/imguiLib/imguiImpl/imgui_impl_android.cpp
        ${VIEWER_IMPORT_DIR}/src/ui/FrontendUI.cpp
        ${VIEWER_IMPORT_DIR}/src/ui/FrontendUI.h
        ${VIEWER_IMPORT_DIR}/src/database/CSqliteDB.cpp
        ${VIEWER_IMPORT_DIR}/src/database/CSqliteDB.h
        ${VIEWER_IMPORT_DIR}/src/minimapGenerator/minimapGenerator.cpp
        ${VIEWER_IMPORT_DIR}/src/screenshots/screenshotMaker.h
        ${VIEWER_IMPORT_DIR}/src/screenshots/screenshotMaker.cpp
        ${VIEWER_IMPORT_DIR}/src/screenshots/lodepng/lodepng.cpp
        ${VIEWER_IMPORT_DIR}/src/screenshots/lodepng/lodepng.h

        ${VIEWER_IMPORT_DIR}/src/exporters/gltfExporter/GLTFExporter.cpp
        ${VIEWER_IMPORT_DIR}/src/exporters/gltfExporter/GLTFExporter.h

        ${VIEWER_IMPORT_DIR}/src/ui/childWindow/mapConstructionWindow.cpp
        ${VIEWER_IMPORT_DIR}/src/ui/childWindow/mapConstructionWindow.h
        ${VIEWER_IMPORT_DIR}/src/ui/imguiLib/groupPanel/groupPanel.cpp
        ${VIEWER_IMPORT_DIR}/src/ui/imguiLib/groupPanel/groupPanel.h


        ${VIEWER_IMPORT_DIR}/src/ui/imguiLib/disablableButton/disablableButton.cpp
        ${VIEWER_IMPORT_DIR}/src/ui/imguiLib/disablableButton/disablableButton.h
        ${VIEWER_IMPORT_DIR}/src/minimapGenerator/storage/CMinimapDataDB.cpp
        ${VIEWER_IMPORT_DIR}/src/minimapGenerator/storage/CMinimapDataDB.h
        ${VIEWER_IMPORT_DIR}/src/minimapGenerator/entities.h
        ${VIEWER_IMPORT_DIR}/src/ui/imguiLib/compactColorPicker/compactColorPicker.cpp
        ${VIEWER_IMPORT_DIR}/src/ui/imguiLib/compactColorPicker/compactColorPicker.h
        ${VIEWER_IMPORT_DIR}/src/ui/imguiLib/imageButton2/imageButton2.cpp
        ${VIEWER_IMPORT_DIR}/src/ui/imguiLib/imageButton2/imageButton2.h
        ${VIEWER_IMPORT_DIR}/src/exporters/dataExporter/DataExporterClass.cpp
        ${VIEWER_IMPORT_DIR}/src/exporters/dataExporter/DataExporterClass.h)

string(TOUPPER ${CMAKE_SYSTEM_NAME} CMAKE_SYSTEM_NAME_LOWERCASE)

#########################################################
# FIND OPENGL
#########################################################
if (NOT CMAKE_SYSTEM_NAME_LOWERCASE MATCHES "ANDROID")
	set(OpenGL_GL_PREFERENCE GLVND)
	find_package(OpenGL REQUIRED)
	include_directories(${OpenGL_INCLUDE_DIRS})
	link_directories(${OpenGL_LIBRARY_DIRS})
	if(NOT OPENGL_FOUND)
		message(ERROR " OPENGL not found!")
	endif(NOT OPENGL_FOUND)
else()
#	if (${ANDROID_PLATFORM_LEVEL} LESS 12)
#		message(FATAL_ERROR "OpenGL 2 is not supported before API level 11 /
#                      (currently using ${ANDROID_PLATFORM_LEVEL}).")
#		return()
#	elseif (${ANDROID_PLATFORM_LEVEL} LESS 18)
		add_definitions("-DDYNAMIC_ES3")
		set(GL3STUB_SRC gl3stub.c)
		set(OPENGL_LIB GLESv3)
#	else ()
#		set(OPENGL_LIB GLESv3)
#	endif (${ANDROID_PLATFORM_LEVEL} LESS 12)
endif()

#Build the executiable
#set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
#set(THREADS_PREFER_PTHREAD_FLAG TRUE)
#find_package( Threads REQUIRED )

#if(CMAKE_USE_PTHREADS_INIT)
#    set(CMAKE_C_FLAGS ${CMAKE_C_FLAGS} "-pthread")
#endif()


add_subdirectory(${VIEWER_IMPORT_DIR}/3rdparty/SQLiteCpp ${CMAKE_CURRENT_BINARY_DIR}/SQLiteCpp)

set(LINK_VULKAN OFF CACHE BOOL "Disable vulkan linking" )

add_library(AWebWoWViewerCpp SHARED ${SOURCE_FILES})
message(ccp WoWViewerLib_INCLUDE_DIRS = ${WoWViewerLib_INCLUDE_DIRS})
target_include_directories(AWebWoWViewerCpp PUBLIC ${WoWViewerLib_INCLUDE_DIRS})
target_include_directories(AWebWoWViewerCpp PUBLIC ${PROJECT_SOURCE_DIR}/WoWViewerLib/3rdparty/mathfu/include)
target_include_directories(AWebWoWViewerCpp PUBLIC ${PROJECT_SOURCE_DIR}/WoWViewerLib/3rdparty/mathfu/include)

#target_compile_options(AWebWoWViewerCpp PRIVATE "-static")
target_compile_definitions(AWebWoWViewerCpp PUBLIC -DIMGUI_USER_CONFIG=<imguiCustomConfig.h>)
target_include_directories(AWebWoWViewerCpp PUBLIC ${VIEWER_IMPORT_DIR}/src/ui/imguiLib)

add_dependencies(AWebWoWViewerCpp WoWViewerLib)
#add_dependencies(AWebWoWViewerCpp storm)
add_dependencies(AWebWoWViewerCpp casc_static)

#TODO: platform dependant!!
check_cxx_compiler_flag(-std=c++17 HAVE_FLAG_STD_CXX17)
if(HAVE_FLAG_STD_CXX17)
	# Have -std=c++17, use it
	message("WOWLIB HAVE_FLAG_STD_CXX17 is supported")
	#    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -std=c++17" )
	#    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -std=c++17" )
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -std=c++17" )
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -std=c++17" )
else()
	check_cxx_compiler_flag(-std=c++1z HAVE_FLAG_STD_CXX1Z)
	if(HAVE_FLAG_STD_CXX1Z)
		#        set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -std=c++1z" )
		#        set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -std=c++1z" )
		set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -std=c++1z")
		set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -std=c++1z")
	else()
		message(ERROR "No supported flags")
	endif()
endif()

if (MSVC)
	include(CheckCXXCompilerFlag)
	CHECK_CXX_COMPILER_FLAG("/std:c++17" _cpp_17_flag_supported)
	message("MSVC Is on")
	if (_cpp_17_flag_supported)
		message("/std:c++17 is supported")
		#target_compile_options(AWebWoWViewerCpp "/std:c++17")
		set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /std:c++17")
		set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /std:c++17")
		set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} /std:c++17")
		#target_compile_options(AWebWoWViewerCpp /std:c++17)
	endif()
endif()

target_link_libraries(AWebWoWViewerCpp ${OPENGL_LIBRARIES})
target_link_libraries(AWebWoWViewerCpp ${OPENGL_LIB})
target_link_libraries(AWebWoWViewerCpp zlibstatic)

target_link_libraries(AWebWoWViewerCpp SQLiteCpp sqlite3)
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND NOT CMAKE_SYSTEM_NAME STREQUAL "Linux" )
	target_link_libraries(AWebWoWViewerCpp ssp)
endif()
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND NOT CMAKE_SYSTEM_NAME STREQUAL "Linux" )
	target_link_libraries(AWebWoWViewerCpp ssp)
endif()
target_link_libraries(AWebWoWViewerCpp WoWViewerLib)

#  target_link_libraries(AWebWoWViewerCpp Threads::Threads)

  target_link_libraries(AWebWoWViewerCpp libEGL)
  target_link_libraries(AWebWoWViewerCpp GLESv2)
  target_link_libraries(AWebWoWViewerCpp android)
  target_link_libraries(AWebWoWViewerCpp log)

#target_link_libraries(AWebWoWViewerCpp polym)
#target_link_libraries(AWebWoWViewerCpp storm)
target_link_libraries(AWebWoWViewerCpp casc_static)

target_link_libraries(AWebWoWViewerCpp png_static)
target_link_libraries(AWebWoWViewerCpp cpr::cpr)

if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	include_directories(AWebWoWViewerCpp "./wowViewerLib/src/opengl")
	target_link_libraries(AWebWoWViewerCpp opengl32)
	target_link_libraries(AWebWoWViewerCpp -lm)
	target_link_libraries(AWebWoWViewerCpp -lwininet)
else()
	target_link_libraries(AWebWoWViewerCpp GL)
endif(${CMAKE_SYSTEM_NAME} MATCHES "Windows")