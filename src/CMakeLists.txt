
find_package(Qt4 REQUIRED)

include(FindPkgConfig)
pkg_check_modules(APR REQUIRED apr-1)
pkg_check_modules(OPENEXR REQUIRED OpenEXR)

find_library(GLEW_LIBRARIES GLEW)

find_library(CG_LIBRARY Cg)
find_library(CG_GL_LIBRARY CgGL)
set(CG_LIBRARIES ${CG_LIBRARY} ${CG_GL_LIBRARY})


set(
    headers
    MainWindow.h
    GLSurface.h
    )

set(
    sources
    main.cpp
    MainWindow.cpp
    GLSurface.cpp
    )

qt4_automoc(${sources})

include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${QT_INCLUDE_DIR}
    ${QT_QTCORE_INCLUDE_DIR}
    ${QT_QTGUI_INCLUDE_DIR}
    ${QT_QTOPENGL_INCLUDE_DIR}
    ${APR_INCLUDE_DIRS}
    ${OPENEXR_INCLUDE_DIRS}
    )

add_executable(gazer ${headers} ${sources})

target_link_libraries(gazer
    ${QT_QTCORE_LIBRARY}
    ${QT_QTGUI_LIBRARY}
    ${QT_QTOPENGL_LIBRARY}
    ${APR_LIBRARIES}
    ${OPENEXR_LIBRARIES}
    ${GLEW_LIBRARIES}
    )

