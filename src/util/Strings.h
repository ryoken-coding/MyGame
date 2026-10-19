// Strings.h - utilitários de texto (formatação e quebra de linhas)
#pragma once

#include <string>
#include <vector>

namespace util {

// Formata com estilo printf e devolve std::string (evita buffers fixos).
std::string formatText(const char* fmt, ...);

// Quebra o texto em linhas que caibam em maxWidth pixels, na fonte dada.
std::vector<std::string> wrapText(const std::string& text, int maxWidth, int fontSize);

} // namespace util
