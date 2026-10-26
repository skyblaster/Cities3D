add_library(json_spirit STATIC
    json_spirit_reader.cpp
    json_spirit_writer.cpp
    json_spirit_value.cpp
)

target_include_directories(json_spirit PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(json_spirit PUBLIC Boost::headers)
target_compile_features(json_spirit PUBLIC cxx_std_17)
target_compile_definitions(json_spirit PUBLIC BOOST_BIND_GLOBAL_PLACEHOLDERS)
set_target_properties(json_spirit PROPERTIES
    POSITION_INDEPENDENT_CODE ON
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
)
if(MSVC)
    target_compile_options(json_spirit PRIVATE /EHsc /GR /wd4244 /wd4267 /wd4996)
endif()
