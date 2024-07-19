cmake_minimum_required(VERSION 3.15)
set(CMAKE_CXX_STANDARD 20)

project(ProjGen)

add_executable(ProjectGenerator
	"Code/main.cpp"
	"Code/Util.cpp"
	"Code/XML/XML.cpp"
	"Code/MSVC/SLN.cpp"
	"Code/MSVC/VSProj.cpp"
)

set_target_properties(ProjectGenerator
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY  "${CMAKE_CURRENT_LIST_DIR}/../../"
)
