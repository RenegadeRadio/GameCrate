cmake_minimum_required(VERSION 3.20)
project(GameCrate LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(gamecrate
    src/launcher/main.cpp
    src/launcher/AppContainerLauncher.cpp
    src/launcher/AclManager.cpp
    src/launcher/ProfileStore.cpp
    src/launcher/FootprintScanner.cpp
    src/launcher/InstallManager.cpp
    src/launcher/VirtualStorage.cpp
    src/launcher/RegistryScanner.cpp
)

target_include_directories(gamecrate PRIVATE src/launcher/include)

target_compile_definitions(gamecrate PRIVATE UNICODE _UNICODE WIN32_LEAN_AND_MEAN)

target_link_libraries(gamecrate PRIVATE
    Userenv
    Advapi32
    Shell32
    onecoreuap
)

if(MSVC)
    target_compile_options(gamecrate PRIVATE /W4 /permissive-)
endif()

install(TARGETS gamecrate RUNTIME DESTINATION bin)
install(DIRECTORY profiles/ DESTINATION share/gamecrate/profiles)
install(DIRECTORY tools/ DESTINATION share/gamecrate/tools)
