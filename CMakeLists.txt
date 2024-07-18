cmake_minimum_required(VERSION 3.15)

project(Klemmgine CXX)

# The engine optionally uses some more modern C++ features.
# But these are behind preprocessor checks that check if the feature is availabe.
if("cxx_std_23" IN_LIST CMAKE_CXX_COMPILE_FEATURES)
	message("Klemmgine: Using C++23")
	set(CMAKE_CXX_STANDARD 23)
else()
	set(CMAKE_CXX_STANDARD 20)
endif()


set(ENGINE_SRC_DIR "EngineSource")

add_subdirectory("Tools/ProjectGenerator")
add_subdirectory("Tools/BuildTool")

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
add_subdirectory("Dependencies/SDL" EXCLUDE_FROM_ALL)
add_subdirectory("Dependencies/SDL_net" EXCLUDE_FROM_ALL)
set(BUILD_SHARED_LIBS OFF)
add_subdirectory("Dependencies/glew-cmake" EXCLUDE_FROM_ALL)
add_subdirectory("Dependencies/glm" EXCLUDE_FROM_ALL)
add_subdirectory("Dependencies/openal-soft" EXCLUDE_FROM_ALL)
add_subdirectory("Dependencies/JoltPhysics/Build" EXCLUDE_FROM_ALL)

if(EDITOR)
	add_subdirectory("Dependencies/assimp")
endif()

add_library(
   Klemmgine STATIC
   ${SRCS}
)

KlemmgineDependsOnBuildTool(Klemmgine "${CMAKE_BINARY_DIR}/Tools/BuildTool/Output" "${CMAKE_CURRENT_LIST_DIR}/EngineSource/Objects" "")

set_target_properties(Klemmgine
	PROPERTIES
	ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
)

if(NOT KLEMMGINE_NO_CSHARP)
	target_compile_definitions(Klemmgine PUBLIC ENGINE_CSHARP)
endif()

if(EDITOR)
	message("EDITOR")
	target_compile_definitions(Klemmgine PUBLIC EDITOR)
elseif(RELEASE)
	target_compile_definitions(Klemmgine PUBLIC RELEASE)
elseif(SERVER)
	target_compile_definitions(Klemmgine PUBLIC SERVER)
endif()

target_include_directories(Klemmgine PUBLIC "${ENGINE_SRC_DIR}")
target_link_libraries(Klemmgine PUBLIC SDL2::SDL2)
target_link_libraries(Klemmgine PUBLIC libglew_static)
target_link_libraries(Klemmgine PUBLIC glm::glm-header-only)
target_link_libraries(Klemmgine PUBLIC OpenAL::OpenAL)
target_link_libraries(Klemmgine PUBLIC SDL2_net::SDL2_net)
target_link_libraries(Klemmgine PUBLIC Jolt)
if(EDITOR)
	target_link_libraries(Klemmgine PUBLIC assimp::assimp)
endif()