cmake_minimum_required(VERSION 3.13)
project($NAME VERSION 1.0
LANGUAGES CXX)


include(ProjectFiles.cmake)

set(Includes ${Includes}
$INCLUDES_LIST
)

set(StaticLibraries ${StaticLibraries}
$STATIC_LIBS
)


add_executable($NAME ${SourceFiles})
set_property(TARGET $NAME PROPERTY CXX_STANDARD 20)

target_include_directories($NAME PRIVATE ${Includes})
target_link_libraries($NAME ${StaticLibraries})
set_target_properties($NAME PROPERTIES RUNTIME_OUTPUT_DIRECTORY $OUTPUT_PATH$<0:>)
