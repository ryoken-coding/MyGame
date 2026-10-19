// Files.h - leitura/escrita de arquivos e localização de recursos do projeto.
#pragma once

#include <string>
#include <vector>

namespace util {

// Resolve um caminho de recurso relativo à raiz do projeto (ex.: "assets/maps/vila.txt").
// Ordem de busca: diretório de trabalho atual e, em seguida, a pasta do executável.
// Se não encontrar, devolve string vazia e preenche *searched com os caminhos testados.
std::string resolveResourcePath(const std::string& relative, std::vector<std::string>* searched = nullptr);

// Leitura/escrita de texto com mensagens de erro claras em `error`.
bool readTextFile(const std::string& path, std::string& out, std::string& error);
bool writeTextFile(const std::string& path, const std::string& content, std::string& error);

} // namespace util
