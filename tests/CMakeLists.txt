cmake_minimum_required(VERSION 3.9.1)
project(MandarinaTest)

if("${PROJECT_SOURCE_DIR}" STREQUAL "${PROJECT_BINARY_DIR}")
	message(FATAL_ERROR "In-source builds are not allowed, use a separate build directory.")
endif()

set(CMAKE_CXX_STANDARD 11)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DMANDARINA_DEBUG")

#required by GameNetworkingSockets (for UNIX systems)
if (UNIX AND NOT APPLE)
	add_definitions(-DPOSIX)

	if (NOT APPLE)
		add_definitions(-DLINUX)
	else()
		add_definitions(-DOSX)
	endif()
endif ()

include_directories(include)

set(SRC_DIR "./src")
file(GLOB SRC_FILES "${SRC_DIR}/*.cpp" "${INCLUDE_DIR}/*.inl")

include_directories(~/Programacion/Librerias/SFML/include)
set(SFML_LIBS debug libsfml-audio-d.so libsfml-graphics-d.so libsfml-network-d.so libsfml-window-d.so libsfml-system-d.so
			  optimized libsfml-audio.so libsfml-graphics.so libsfml-network.so libsfml-window.so libsfml-system.so)
link_directories(~/Programacion/Librerias/SFML/lib)

include_directories(~/Programacion/Librerias/rapidjson/include)

include_directories(~/Programacion/Librerias/GameNetworkingSockets/include)
set(NETWORK_LIBS libGameNetworkingSockets.so)
link_directories(~/Programacion/Librerias/GameNetworkingSockets/lib)

add_executable(mandarina_tests "tests_main.cpp")
target_link_libraries(mandarina_tests stdc++fs ${SFML_LIBS} libGL.so ${NETWORK_LIBS} libssl.so libcrypto.so pthread)