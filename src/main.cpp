// main.cpp - ponto de entrada: janela, laço principal e modo de verificação (--smoke).
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

#include "raylib.h"

#include "core/Game.h"

namespace {

constexpr int kScreenWidth = 1024;
constexpr int kScreenHeight = 576;

// Tela de erro usada quando o jogo não consegue iniciar (ex.: mapa ausente).
void showErrorScreen(const std::string& message)
{
    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(Color{30, 12, 16, 255});
        DrawText("Não foi possível iniciar o jogo:", 40, 80, 28, Color{240, 160, 150, 255});

        int y = 130;
        for (const char* line = message.c_str(); *line != '\0';) {
            // Quebra simples para caber na janela.
            char buffer[96];
            std::snprintf(buffer, sizeof(buffer), "%.88s", line);
            DrawText(buffer, 40, y, 20, Color{235, 210, 205, 255});
            line += std::strlen(buffer);
            y += 26;
        }
        DrawText("Verifique o README.md e execute o jogo a partir da raiz do projeto.", 40, y + 12, 18,
                 Color{200, 190, 185, 255});
        DrawText("Pressione qualquer tecla para sair.", 40, y + 48, 18, Color{180, 170, 165, 255});
        EndDrawing();

        if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) break;
    }
}

// Modo de verificação: inicia uma partida, roda N quadros, exercita save/load e sai.
// Útil para testar o jogo em máquinas sem janela (CI). Uso: mygame --smoke 120
int runSmokeTest(int frames)
{
    Game game;
    std::string error;
    if (!game.init(error)) {
        std::fprintf(stderr, "[smoke] falha na inicialização: %s\n", error.c_str());
        return 1;
    }
    if (!game.startNewGame(error)) {
        std::fprintf(stderr, "[smoke] falha ao iniciar nova partida: %s\n", error.c_str());
        return 1;
    }
    std::printf("[smoke] nova partida iniciada; rodando %d quadros...\n", frames);

    for (int i = 0; i < frames; i++) {
        game.update(1.0f / 60.0f);
        BeginDrawing();
        game.draw((float)i / 60.0f);
        EndDrawing();
    }

    if (!game.saveGame(error)) {
        std::fprintf(stderr, "[smoke] falha ao salvar: %s\n", error.c_str());
        return 1;
    }
    std::printf("[smoke] save gravado em '%s'.\n", SaveSystem::defaultPath());

    if (!game.loadGame(error)) {
        std::fprintf(stderr, "[smoke] falha ao carregar: %s\n", error.c_str());
        return 1;
    }
    std::printf("[smoke] save carregado com sucesso.\n");

    for (int i = 0; i < 30; i++) {
        game.update(1.0f / 60.0f);
        BeginDrawing();
        game.draw((float)i / 60.0f);
        EndDrawing();
    }

    game.shutdown();
    std::printf("[smoke] verificação concluída sem erros.\n");
    return 0;
}

} // namespace

int main(int argc, char** argv)
{
    // Opções de linha de comando.
    int smokeFrames = 0;
    for (int i = 1; i < argc; i++) {
        if (std::strcmp(argv[i], "--smoke") == 0 && i + 1 < argc) {
            smokeFrames = std::atoi(argv[++i]);
        } else if (std::strcmp(argv[i], "--help") == 0 || std::strcmp(argv[i], "-h") == 0) {
            std::printf("Uso: %s [--smoke N]\n", argv[0]);
            std::printf("  --smoke N   roda N quadros de verificação e sai (sem interação).\n");
            return 0;
        }
    }

    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
    InitWindow(kScreenWidth, kScreenHeight, "Eldara: A Flor de Lumina");
    SetTargetFPS(60);
    InitAudioDevice();

    int exitCode = 0;
    if (smokeFrames > 0) {
        SetTraceLogLevel(LOG_INFO);
        exitCode = runSmokeTest(smokeFrames);
    } else {
        Game game;
        std::string error;
        if (!game.init(error)) {
            showErrorScreen(error);
            exitCode = 1;
        } else {
            while (!WindowShouldClose() && !game.wantsQuit()) {
                game.update(GetFrameTime());
                BeginDrawing();
                game.draw((float)GetTime());
                EndDrawing();
            }
            game.shutdown();
        }
    }

    CloseAudioDevice();
    CloseWindow();
    return exitCode;
}
