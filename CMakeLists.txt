cmake_minimum_required(VERSION 3.15)

project(Klemmgine)

set(CMAKE_CXX_STANDARD 20)

# The engine optionally uses some more modern C++ features.
# But these are behind preprocessor checks that check if the feature is availabe.
if("cxx_std_23" IN_LIST CMAKE_CXX_COMPILE_FEATURES)
	message(STATUS "Klemmgine: Using C++23")
	set(CMAKE_CXX_STANDARD 23)
endif()

if(WIN32)
	file(COPY "${CMAKE_CURRENT_LIST_DIR}/CSharp/lib/nethost.dll" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")
else()
	file(COPY "${CMAKE_CURRENT_LIST_DIR}/CSharp/lib/libnethost.so" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")
endif()

set(ENGINE_SRC_DIR "${CMAKE_CURRENT_LIST_DIR}/EngineSource")

add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/Tools/BuildTool" EXCLUDE_FROM_ALL "BuildTool")
add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/Tools/ProjectGenerator" "ProjectGenerator")

file(
	GLOB_RECURSE
	SRCS
	"${ENGINE_SRC_DIR}/**.cpp"
)

option(KLEMMGINE_NO_CSHARP OFF)
option(RELEASE OFF)
option(EDITOR OFF)
option(SERVER OFF)

set(glew-cmake_BUILD_SHARED OFF)
set(ONLY_LIBS ON)
set(USE_STATIC_MSVC_RUNTIME_LIBRARY OFF)
set(DENABLE_ALL_WARNINGS OFF)
set(ASSIMP_WARNINGS_AS_ERRORS OFF)
add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/Dependencies/SDL" EXCLUDE_FROM_ALL "deps/SDL")
add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/Dependencies/SDL_net" EXCLUDE_FROM_ALL "deps/SDL_net")
set(BUILD_SHARED_LIBS OFF)
add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/Dependencies/glew-cmake" EXCLUDE_FROM_ALL "deps/glew")
add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/Dependencies/glm" EXCLUDE_FROM_ALL "deps/glm")
add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/Dependencies/openal-soft" EXCLUDE_FROM_ALL "deps/openal")
add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/Dependencies/JoltPhysics/Build" EXCLUDE_FROM_ALL "deps/jolt")

if(EDITOR)
	add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/Dependencies/assimp" EXCLUDE_FROM_ALL "deps/assimp")
endif()

add_library(
   LibKlemmgine STATIC EXCLUDE_FROM_ALL
   ${SRCS}
)

if(NOT KLEMMGINE_NO_CSHARP)
	# Build C# libraries
	message(STATUS "Building C#: Engine.Core")
	execute_process(COMMAND dotnet build WORKING_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/CSharp/Core" OUTPUT_QUIET)
	message(STATUS "Building C#: Engine")
	execute_process(COMMAND dotnet build WORKING_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/CSharp/Engine" OUTPUT_QUIET)
endif()

KlemmgineDependsOnBuildTool(LibKlemmgine "${CMAKE_BINARY_DIR}/Tools/BuildTool/Output" "${CMAKE_CURRENT_LIST_DIR}/EngineSource/Objects" "")

# Makes the Linux linker search for shared libraries in the directory where the executable is located.
if (NOT WIN32)
	add_link_options("-Wl,-rpath,'$ORIGIN'")
endif()

if(NOT KLEMMGINE_NO_CSHARP)
	target_compile_definitions(LibKlemmgine PUBLIC ENGINE_CSHARP)
	if(WIN32)
		target_link_libraries(LibKlemmgine PUBLIC "${CMAKE_CURRENT_LIST_DIR}/CSharp/lib/nethost.lib")
	else()
		target_link_libraries(LibKlemmgine PUBLIC "${CMAKE_CURRENT_LIST_DIR}/CSharp/lib/libnethost.so")
	endif()
endif()

if(EDITOR)
	target_compile_definitions(LibKlemmgine PUBLIC EDITOR)
	message("Building with EDITOR engine configuration")
elseif(RELEASE)
	target_compile_definitions(LibKlemmgine PUBLIC RELEASE)
	message("Building with RELEASE engine configuration")
elseif(SERVER)
	target_compile_definitions(LibKlemmgine PUBLIC SERVER)
	message("Building with SERVER engine configuration")
else()
	message("Building with DEBUG engine configuration")
endif()

target_include_directories(LibKlemmgine PUBLIC "${ENGINE_SRC_DIR}")
target_link_libraries(LibKlemmgine PUBLIC SDL2::SDL2)
target_link_libraries(LibKlemmgine PUBLIC libglew_static)
target_link_libraries(LibKlemmgine PUBLIC glm::glm-header-only)
target_link_libraries(LibKlemmgine PUBLIC OpenAL::OpenAL)
target_link_libraries(LibKlemmgine PUBLIC SDL2_net::SDL2_net)
target_link_libraries(LibKlemmgine PUBLIC Jolt)
if(EDITOR)
	target_link_libraries(LibKlemmgine PUBLIC assimp::assimp)
endif()