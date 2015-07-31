file(GLOB_RECURSE PipelineCpp_src_files *.cpp)

add_library(PipelineCpp STATIC ${PipelineCpp_src_files})
set_target_properties(PipelineCpp PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${DEFAULT_LINK_DIR})
target_link_libraries(PipelineCpp)
