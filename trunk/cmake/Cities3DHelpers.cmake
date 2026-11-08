function(cities3d_apply_common target)
    target_compile_features(${target} PUBLIC cxx_std_17)
    target_include_directories(${target} PUBLIC
        "${CITIES3D_SRC_DIR}/common"
        "${CITIES3D_SRC_DIR}"
    )
    target_compile_definitions(${target} PUBLIC
        BOOST_BIND_GLOBAL_PLACEHOLDERS
        $<$<CONFIG:Debug>:_DEBUG>
        $<$<NOT:$<CONFIG:Debug>>:NDEBUG>
    )
    if(MSVC)
        target_compile_definitions(${target} PUBLIC
            UNICODE
            _UNICODE
            _CRT_SECURE_NO_WARNINGS
            NOMINMAX
            WIN32_LEAN_AND_MEAN
        )
        target_compile_options(${target} PRIVATE
            /W3
            /EHsc
            /GR
            /wd4251
            /wd4275
            /wd4100
            /wd4244
            /wd4267
            /wd4996
        )
    else()
        target_compile_options(${target} PRIVATE
            -Wall
            -Wno-unused-parameter
            -Wno-deprecated-declarations
        )
    endif()
endfunction()

function(cities3d_add_core name export_def extra_includes)
    set(_srcs
        "${CITIES3D_SRC_DIR}/${name}/${name}.bulkbuild.cpp"
        "${CITIES3D_SRC_DIR}/${name}/stdwx.cpp"
    )
    add_library(${name} SHARED ${_srcs})
    cities3d_apply_common(${name})
    target_compile_definitions(${name} PRIVATE ${export_def})
    target_include_directories(${name} PUBLIC
        "${CITIES3D_SRC_DIR}/${name}"
        ${extra_includes}
    )
    target_precompile_headers(${name} PRIVATE "${CITIES3D_SRC_DIR}/${name}/stdwx.h")
    set_target_properties(${name} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CITIES3D_DIST_DIR}"
        LIBRARY_OUTPUT_DIRECTORY "${CITIES3D_DIST_DIR}"
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
        PREFIX ""
    )
    if(UNIX AND NOT APPLE)
        set_target_properties(${name} PROPERTIES
            BUILD_RPATH "$ORIGIN"
            INSTALL_RPATH "$ORIGIN"
        )
    endif()
endfunction()

function(cities3d_add_plugin name)
    set(_srcs
        "${CITIES3D_SRC_DIR}/${name}/${name}.bulkbuild.cpp"
        "${CITIES3D_SRC_DIR}/${name}/stdwx.cpp"
        "${CITIES3D_SRC_DIR}/common/RuleSetDatabase.cpp"
    )
    if(EXISTS "${CITIES3D_SRC_DIR}/common/OptionDatabase.cpp")
        # OptionDatabase is compiled into ruleset plugins that register options.
        list(APPEND _srcs "${CITIES3D_SRC_DIR}/common/OptionDatabase.cpp")
    endif()
    add_library(${name} MODULE ${_srcs})
    cities3d_apply_common(${name})
    target_include_directories(${name} PRIVATE
        "${CITIES3D_SRC_DIR}/${name}"
        "${CITIES3D_SRC_DIR}/DatabaseCore"
        "${CITIES3D_SRC_DIR}/ModelCore"
        "${CITIES3D_SRC_DIR}/RulesCore"
        "${CITIES3D_SRC_DIR}/GLCore"
        "${CITIES3D_SRC_DIR}/UICore"
    )
    target_precompile_headers(${name} PRIVATE "${CITIES3D_SRC_DIR}/${name}/stdwx.h")
    set_target_properties(${name} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CITIES3D_PLUGIN_DIR_DEBUG}"
        RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CITIES3D_PLUGIN_DIR_RELEASE}"
        RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CITIES3D_PLUGIN_DIR_RELEASE}"
        RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${CITIES3D_PLUGIN_DIR_RELEASE}"
        LIBRARY_OUTPUT_DIRECTORY_DEBUG "${CITIES3D_PLUGIN_DIR_DEBUG}"
        LIBRARY_OUTPUT_DIRECTORY_RELEASE "${CITIES3D_PLUGIN_DIR_RELEASE}"
        LIBRARY_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CITIES3D_PLUGIN_DIR_RELEASE}"
        LIBRARY_OUTPUT_DIRECTORY_MINSIZEREL "${CITIES3D_PLUGIN_DIR_RELEASE}"
        PREFIX ""
    )
    if(NOT WIN32)
        set_target_properties(${name} PROPERTIES
            LIBRARY_OUTPUT_DIRECTORY "${CITIES3D_PLUGIN_DIR_RELEASE}"
        )
    endif()
    if(UNIX AND NOT APPLE)
        # Plugins live in plugins/; libwx sits one directory up next to Cities3D.
        set_target_properties(${name} PROPERTIES
            BUILD_RPATH "$ORIGIN:$ORIGIN/.."
            INSTALL_RPATH "$ORIGIN:$ORIGIN/.."
        )
    endif()
endfunction()
