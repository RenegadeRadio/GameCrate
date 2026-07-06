cmake_minimum_required(VERSION 3.20)
project(WinDoze LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(windoze
    src/launcher/main.cpp
    src/launcher/AppContainerLauncher.cpp
    src/launcher/AclManager.cpp
    src/launcher/ProfileStore.cpp
)

target_include_directories(windoze PRIVATE src/launcher/include)

target_compile_definitions(windoze PRIVATE UNICODE _UNICODE WIN32_LEAN_AND_MEAN)

target_link_libraries(windoze PRIVATE
    Userenv
    Advapi32
    Shell32
    onecoreuap
)

if(MSVC)
    target_compile_options(windoze PRIVATE /W4 /permissive-)
endif()

install(TARGETS windoze RUNTIME DESTINATION bin)
install(DIRECTORY profiles/ DESTINATION share/windoze/profiles)
install(DIRECTORY tools/ DESTINATION share/windoze/tools)
