
find_package(Doxygen)

if (NOT WIN32)
    set(DEST share/)
endif ()

if (DOT)
    set(HAVE_DOT "YES")
else ()
    set(HAVE_DOT "NO")
endif ()

execute_process(
    COMMAND ./versionfilter.sh
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    OUTPUT_VARIABLE PROJECT_NUMBER
    )

add_custom_target(doc)

if (DOXYGEN_FOUND)
    #add_custom_target(all_doc COMMAND echo blah)
    add_custom_target(pdf)

    #add_custom_command(TARGET doc DEPENDS all_doc)
    #add_custom_target(doc DEPENDS all_doc)
    #add_custom_target(doc DEPENDS all_doc)

    foreach(src gazer)
        configure_file(
            ${CMAKE_CURRENT_SOURCE_DIR}/${src}.dox.in
            ${CMAKE_CURRENT_BINARY_DIR}/${src}.dox
            )
        file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${src})
        add_custom_target(${src}_doc
            COMMAND ${DOXYGEN} ${CMAKE_CURRENT_BINARY_DIR}/${src}.dox
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            )
        add_dependencies(doc ${src}_doc)
        if (CMAKE_MAKE_PROGRAM)
            add_custom_target(${src}_pdf
                COMMAND ${CMAKE_MAKE_PROGRAM} -s -C ${CMAKE_CURRENT_BINARY_DIR}/${src}/latex
                #DEPENDS ${src}_doc
                )
            add_dependencies(pdf ${src}_pdf)
        endif()
        if (GAS_ENABLE_INSTALLER)
            install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${src}
                DESTINATION ${DEST}doc)
        endif()
    endforeach()
endif ()
