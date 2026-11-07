if("${CONFIG}" STREQUAL "Debug")
    return()
endif()

if(NOT DEST)
    message(FATAL_ERROR "DEST is required")
endif()

if(NOT EXISTS "${DLL_LIST_FILE}")
    return()
endif()

file(STRINGS "${DLL_LIST_FILE}" _dlls)
foreach(_dll ${_dlls})
    if(EXISTS "${_dll}")
        file(COPY "${_dll}" DESTINATION "${DEST}")
    endif()
endforeach()
