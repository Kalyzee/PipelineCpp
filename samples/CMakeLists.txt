#/*
#-----------------------------------------------------------------------------
#Filename:    CMakeLists.txt
#-----------------------------------------------------------------------------
#*/

file(GLOB samples_src_files main.cpp)

add_executable(sample WIN32 ${samples_src_files})
set_target_properties(sample PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${DEFAULT_BIN_DIR} OUTPUT_NAME sample)
target_link_libraries(sample PipelineCpp ${Boost_LIBRARIES})
