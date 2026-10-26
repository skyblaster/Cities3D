if(NOT DEFINED PLUGIN_DIR)
    message(FATAL_ERROR "PLUGIN_DIR is required")
endif()

file(GLOB plugin_files "${PLUGIN_DIR}/*")
foreach(plugin_file ${plugin_files})
    get_filename_component(file_name "${plugin_file}" NAME)
    if(NOT file_name MATCHES "Rules\\.(dll|so|dylib)$"
       AND NOT file_name MATCHES "Rules\\.(pdb|ilk)$")
        file(REMOVE "${plugin_file}")
    endif()
endforeach()
