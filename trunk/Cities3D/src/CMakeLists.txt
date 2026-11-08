set(CITIES3D_WX_LIBS wx::core wx::base wx::net wx::xml wx::adv wx::aui wx::gl wx::html)

cities3d_add_core(DatabaseCore DATABASE_EXPORT "")
target_link_libraries(DatabaseCore PUBLIC ${CITIES3D_WX_LIBS} Boost::headers)
if(UNIX AND NOT APPLE)
    # Static wxGTK can leave pcre2-8 / dbus / atspi unresolved in our shared
    # cores. Link them when pkg-config can see them (Fedora/Homebrew hosts);
    # Flatpak's Freedesktop SDK usually provides them too.
    find_package(PkgConfig)
    if(PkgConfig_FOUND)
        pkg_check_modules(CITIES3D_WXGTK_EXTRA IMPORTED_TARGET
            libpcre2-8
            dbus-1
            atspi-2
        )
        if(CITIES3D_WXGTK_EXTRA_FOUND)
            target_link_libraries(DatabaseCore PUBLIC PkgConfig::CITIES3D_WXGTK_EXTRA)
        endif()
    endif()
endif()

cities3d_add_core(ModelCore MODEL_EXPORT "${CITIES3D_SRC_DIR}/DatabaseCore")
target_link_libraries(ModelCore PUBLIC DatabaseCore)

cities3d_add_core(RulesCore RULES_EXPORT "${CITIES3D_SRC_DIR}/json_spirit")
target_link_libraries(RulesCore PUBLIC DatabaseCore ModelCore json_spirit ${CITIES3D_WX_LIBS})
if(MSVC)
    target_link_options(RulesCore PRIVATE /FORCE:MULTIPLE)
endif()
if(CITIES3D_USE_MINIAUDIO)
    find_path(MINIAUDIO_INCLUDE_DIR miniaudio.h REQUIRED)
    target_include_directories(RulesCore PRIVATE "${MINIAUDIO_INCLUDE_DIR}")
    target_compile_definitions(RulesCore PRIVATE CITIES3D_USE_MINIAUDIO)
    if(WIN32)
        target_link_libraries(RulesCore PUBLIC ole32 winmm)
    elseif(APPLE)
        target_link_libraries(RulesCore PUBLIC "-framework CoreAudio" "-framework AudioToolbox")
    else()
        target_link_libraries(RulesCore PUBLIC pthread dl m)
    endif()
endif()

cities3d_add_core(GLCore GL_EXPORT "${CITIES3D_SRC_DIR}/RulesCore")
target_link_libraries(GLCore PUBLIC RulesCore OpenGL::GL GLEW::GLEW wx::gl)
if(TARGET OpenGL::GLU)
    target_link_libraries(GLCore PUBLIC OpenGL::GLU)
elseif(WIN32)
    target_link_libraries(GLCore PUBLIC glu32)
endif()
target_compile_definitions(GLCore PUBLIC GLEW_NO_GLU)
target_include_directories(GLCore PUBLIC ${OPENGL_INCLUDE_DIR})

cities3d_add_core(UICore UI_EXPORT "${CITIES3D_SRC_DIR}/RulesCore")
target_link_libraries(UICore PUBLIC DatabaseCore ModelCore RulesCore ${CITIES3D_WX_LIBS})

cities3d_add_plugin(CoreRules)
set(CITIES3D_GL_LIBS OpenGL::GL)
if(TARGET OpenGL::GLU)
    list(APPEND CITIES3D_GL_LIBS OpenGL::GLU)
elseif(WIN32)
    list(APPEND CITIES3D_GL_LIBS glu32)
endif()

target_link_libraries(CoreRules PRIVATE DatabaseCore ModelCore RulesCore GLCore UICore json_spirit ${CITIES3D_GL_LIBS} ${CITIES3D_WX_LIBS})
if(MSVC)
    target_link_options(CoreRules PRIVATE /FORCE:MULTIPLE)
endif()

cities3d_add_plugin(StandardRules)
target_link_libraries(StandardRules PRIVATE DatabaseCore ModelCore RulesCore GLCore UICore ${CITIES3D_GL_LIBS} ${CITIES3D_WX_LIBS})

cities3d_add_plugin(SeafarersRules)
target_link_libraries(SeafarersRules PRIVATE DatabaseCore ModelCore RulesCore GLCore UICore ${CITIES3D_GL_LIBS} ${CITIES3D_WX_LIBS})

cities3d_add_plugin(CitiesRules)
target_link_libraries(CitiesRules PRIVATE DatabaseCore ModelCore RulesCore GLCore UICore ${CITIES3D_GL_LIBS} ${CITIES3D_WX_LIBS})

cities3d_add_plugin(VolcanoRules)
target_link_libraries(VolcanoRules PRIVATE DatabaseCore ModelCore RulesCore GLCore UICore ${CITIES3D_WX_LIBS})

cities3d_add_plugin(NetworkRules)
target_link_libraries(NetworkRules PRIVATE DatabaseCore ModelCore RulesCore GLCore UICore ${CITIES3D_WX_LIBS})

add_executable(Cities3D WIN32
    "${CITIES3D_SRC_DIR}/Cities3D/Cities3D.bulkbuild.cpp"
    "${CITIES3D_SRC_DIR}/Cities3D/stdwx.cpp"
)
cities3d_apply_common(Cities3D)
target_include_directories(Cities3D PRIVATE
    "${CITIES3D_SRC_DIR}/Cities3D"
    "${CITIES3D_SRC_DIR}/DatabaseCore"
    "${CITIES3D_SRC_DIR}/ModelCore"
    "${CITIES3D_SRC_DIR}/RulesCore"
    "${CITIES3D_SRC_DIR}/GLCore"
    "${CITIES3D_SRC_DIR}/UICore"
)
target_precompile_headers(Cities3D PRIVATE "${CITIES3D_SRC_DIR}/Cities3D/stdwx.h")
target_link_libraries(Cities3D PRIVATE
    DatabaseCore UICore ModelCore GLCore RulesCore
    ${CITIES3D_GL_LIBS}
    ${CITIES3D_WX_LIBS}
)
set_target_properties(Cities3D PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CITIES3D_DIST_DIR}"
    VS_DEBUGGER_WORKING_DIRECTORY "${CITIES3D_DIST_DIR}"
)
if(MSVC)
    # Plugin DLLs run hundreds of static registrars during LoadLibrary; the
    # default 1 MB stack is enough for an isolated LoadLibrary but not for
    # FindPlugins nested in wx/app startup (error 1114 on StandardRules).
    target_link_options(Cities3D PRIVATE /STACK:0x800000)
endif()
if(WIN32)
    target_sources(Cities3D PRIVATE "${CITIES3D_SRC_DIR}/Cities3D/cities3d.rc")
endif()

add_dependencies(Cities3D
    CoreRules StandardRules SeafarersRules CitiesRules VolcanoRules NetworkRules
)
# vcpkg applocal copies wx/runtime DLLs next to plugins; remove them after
# every plugin link so LoadLibrary does not initialize a second wxWidgets.
add_custom_command(TARGET Cities3D POST_BUILD
    COMMAND ${CMAKE_COMMAND}
        -DPLUGIN_DIR=$<IF:$<CONFIG:Debug>,${CITIES3D_PLUGIN_DIR_DEBUG},${CITIES3D_PLUGIN_DIR_RELEASE}>
        -P "${CMAKE_SOURCE_DIR}/cmake/CleanPluginDir.cmake"
    COMMENT "Removing non-plugin libraries from the plugin directory"
)
if(MSVC)
    set(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_SKIP TRUE)
    include(InstallRequiredSystemLibraries)
    set(_vc_redist_list "${CMAKE_BINARY_DIR}/vc_redist_dlls.txt")
    file(WRITE "${_vc_redist_list}" "")
    foreach(_dll ${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS})
        file(APPEND "${_vc_redist_list}" "${_dll}\n")
    endforeach()
    add_custom_command(TARGET Cities3D POST_BUILD
        COMMAND ${CMAKE_COMMAND}
            -DCONFIG=$<CONFIG>
            -DDEST=$<TARGET_FILE_DIR:Cities3D>
            -DDLL_LIST_FILE=${_vc_redist_list}
            -P "${CMAKE_SOURCE_DIR}/cmake/CopyVcRedist.cmake"
        COMMENT "Copying the Visual C++ runtime next to Cities3D.exe"
    )
endif()
