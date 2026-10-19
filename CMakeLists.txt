cmake_minimum_required(VERSION 3.22)
project(MyGame VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Plataforma do raylib. "Desktop" é o normal; "Memory" gera um build headless
# útil para verificação em CI (sem janela, sem entrada).
set(PLATFORM "Desktop" CACHE STRING "Plataforma do raylib (Desktop, Memory, Web, ...)")

if(MSVC)
    add_compile_options(/W4 /utf-8)
else()
    add_compile_options(-Wall -Wextra)
endif()

# ------------------------------------------------------------------ raylib
# Tenta um raylib instalado no sistema (vcpkg, apt, homebrew...);
# se não encontrar, baixa automaticamente via FetchContent.
find_package(raylib 6.0 QUIET)
if(NOT raylib_FOUND)
    message(STATUS "raylib não encontrado no sistema; baixando via FetchContent...")
    include(FetchContent)
    FetchContent_Declare(
        raylib
        GIT_REPOSITORY https://github.com/raysan5/raylib.git
        GIT_TAG 6.2
        GIT_SHALLOW TRUE
    )
    set(BUILD_EXAMPLES OFF CACHE INTERNAL "")
    FetchContent_MakeAvailable(raylib)
endif()

# ------------------------------------------------------------------ mygame
add_executable(mygame
    src/main.cpp
    src/core/Game.cpp
    src/core/SaveSystem.cpp
    src/util/Json.cpp
    src/util/Strings.cpp
    src/util/Files.cpp
    src/entities/Entity.cpp
    src/entities/Player.cpp
    src/entities/Enemy.cpp
    src/entities/Npc.cpp
    src/entities/ItemPickup.cpp
    src/world/TileMap.cpp
    src/world/Areas.cpp
    src/world/World.cpp
    src/systems/Inventory.cpp
    src/systems/Quest.cpp
    src/systems/Dialogue.cpp
    src/systems/Combat.cpp
    src/ui/MessageLog.cpp
    src/ui/Hud.cpp
    src/audio/SoundBank.cpp
)

target_include_directories(mygame PRIVATE src)
target_link_libraries(mygame PRIVATE raylib)

# Copia os recursos para junto do executável (permite rodar de fora da raiz também).
add_custom_command(TARGET mygame POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/assets $<TARGET_FILE_DIR:mygame>/assets
    COMMENT "Copiando assets para o diretório do executável"
)
