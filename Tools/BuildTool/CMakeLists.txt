cmake_minimum_required(VERSION 3.15)
set(CMAKE_CXX_STANDARD 20)

project(KlemmgineBuildTool)

add_executable(HeaderGenerate "Code/BuildTool.cpp" "Code/Log.cpp" "Code/ParseFile.cpp")

macro(KlemmgineDependsOnBuildTool ProjectName OutDir InDir1 InDir2)
	
	add_custom_target(
		${ProjectName}_WriteHeaders
		COMMENT "Running build command."
	)

	set(BUILD_TOOL_ARGS in=${InDir1})
	if(NOT ${InDir2} STREQUAL "")
		set(BUILD_TOOL_ARGS ${BUILD_TOOL_ARGS} " " in=${InDir2})
	endif()
	set(BUILD_TOOL_ARGS ${BUILD_TOOL_ARGS} " " out=${OutDir})

	if(KLEMMGINE_NO_CSHARP)
		set(BUILD_TOOL_ARGS ${BUILD_TOOL_ARGS} " " noCSharp)
	endif()

	message(${BUILD_TOOL_ARGS})

	add_custom_command(
		TARGET ${ProjectName}_WriteHeaders
		COMMAND $<TARGET_FILE:HeaderGenerate> ${BUILD_TOOL_ARGS}
	)

	add_dependencies(${ProjectName}_WriteHeaders HeaderGenerate)
	add_dependencies(${ProjectName} ${ProjectName}_WriteHeaders)

	target_include_directories(${ProjectName} PRIVATE ${OutDir})
endmacro()