
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)
install(IMPORTED_RUNTIME_ARTIFACTS ${RUNTIME_ARTIFACT_TARGETS})

set(CPACK_PACKAGE_NAME "gvox-cli")
set(CPACK_PACKAGE_VENDOR "Gabe-Rundlett")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "gvox-cli is an app developed by Gabe Rundlett")
set(CPACK_PACKAGE_DESCRIPTION "gvox-cli can convert between common voxel file formats")
set(CPACK_RESOURCE_FILE_WELCOME "${CMAKE_SOURCE_DIR}/packaging/infos/welcome.txt")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/packaging/infos/license.txt")
set(CPACK_RESOURCE_FILE_README "${CMAKE_SOURCE_DIR}/packaging/infos/readme.txt")
set(CPACK_PACKAGE_ICON "${CMAKE_SOURCE_DIR}/appicon.png")

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    configure_file("packaging/main.rc.in" "${CMAKE_BINARY_DIR}/main.rc")
    target_sources(${PROJECT_NAME} PRIVATE "${CMAKE_BINARY_DIR}/main.rc")

    set(CPACK_GENERATOR WIX)
    set(CPACK_WIX_UPGRADE_GUID 70FA6521-1E61-411B-9782-59DFCD635C5E)
    set(CPACK_PACKAGE_EXECUTABLES ${PROJECT_NAME} "gvox-cli")
    set(CPACK_WIX_PRODUCT_ICON "${CMAKE_SOURCE_DIR}/appicon.png")

    # Set the default installation directory. In this case it becomes C:/Program Files/GabeRundlett
    set(CPACK_PACKAGE_INSTALL_DIRECTORY "GabeRundlett")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    # TODO: Find a better way to package, though tar.gz works for now
    # install(FILES "${CMAKE_SOURCE_DIR}/packaging/gabe_voxel_game.desktop" DESTINATION share/applications)
    # set(CPACK_BINARY_AppImage ON)
endif()

include(InstallRequiredSystemLibraries)
include(CPack)
