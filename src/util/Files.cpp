#include "util/Files.h"

#include <filesystem>
#include <fstream>
#include <sstream>

#include "raylib.h"
#include "util/Strings.h"

namespace util {

std::string resolveResourcePath(const std::string& relative, std::vector<std::string>* searched)
{
    namespace fs = std::filesystem;

    std::vector<std::string> candidates;
    candidates.push_back(relative); // relativo ao diretório de trabalho (raiz do projeto)

    const char* appDir = GetApplicationDirectory();
    if (appDir != nullptr && appDir[0] != '\0') {
        candidates.push_back((fs::path(appDir) / relative).string());
    }

    for (const std::string& candidate : candidates) {
        std::error_code ec;
        if (fs::exists(candidate, ec)) return candidate;
    }

    if (searched != nullptr) *searched = candidates;
    return "";
}

bool readTextFile(const std::string& path, std::string& out, std::string& error)
{
    error.clear();
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) {
        error = formatText("Não foi possível abrir o arquivo para leitura: %s", path.c_str());
        return false;
    }
    std::ostringstream buffer;
    buffer << file.rdbuf();
    if (file.bad()) {
        error = formatText("Erro de leitura do arquivo: %s", path.c_str());
        return false;
    }
    out = buffer.str();
    return true;
}

bool writeTextFile(const std::string& path, const std::string& content, std::string& error)
{
    error.clear();

    // Garante que o diretório de destino exista (ex.: "saves/").
    namespace fs = std::filesystem;
    const fs::path parent = fs::path(path).parent_path();
    if (!parent.empty()) {
        std::error_code ec;
        fs::create_directories(parent, ec);
        if (ec) {
            error = formatText("Não foi possível criar o diretório %s: %s",
                               parent.string().c_str(), ec.message().c_str());
            return false;
        }
    }

    std::ofstream file(path, std::ios::binary | std::ios::trunc);
    if (!file.is_open()) {
        error = formatText("Não foi possível abrir o arquivo para escrita: %s", path.c_str());
        return false;
    }
    file << content;
    if (!file.good()) {
        error = formatText("Erro ao gravar o arquivo: %s", path.c_str());
        return false;
    }
    return true;
}

} // namespace util
