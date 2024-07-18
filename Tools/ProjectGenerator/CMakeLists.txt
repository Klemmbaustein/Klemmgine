
project(KlemmgineProjectGenerator)

add_executable(ProjectGenerator
	"Code/main.cpp"
	"Code/Util.cpp"
	"Code/XML/XML.cpp"
	"Code/MSVC/SLN.cpp"
	"Code/MSVC/VSProj.cpp"
)

set_target_properties(ProjectGenerator
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY  "${CMAKE_SOURCE_DIR}/"
)
