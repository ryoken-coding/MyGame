#include "util/Strings.h"

#include <cstdarg>
#include <cstdio>

#include "raylib.h"

namespace util {

std::string formatText(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    va_list argsCopy;
    va_copy(argsCopy, args);
    const int needed = std::vsnprintf(nullptr, 0, fmt, args);
    va_end(args);

    std::string result;
    if (needed > 0) {
        result.resize(static_cast<size_t>(needed));
        std::vsnprintf(result.data(), result.size() + 1, fmt, argsCopy);
    }
    va_end(argsCopy);
    return result;
}

std::vector<std::string> wrapText(const std::string& text, int maxWidth, int fontSize)
{
    std::vector<std::string> lines;
    std::string current;

    size_t i = 0;
    while (i < text.size()) {
        // Coleta a próxima palavra (ou trecho longo sem espaços).
        size_t start = i;
        while (i < text.size() && text[i] != ' ' && text[i] != '\n') i++;
        std::string word = text.substr(start, i - start);
        const bool isBreak = (i < text.size() && text[i] == '\n');
        if (i < text.size()) i++; // consome o espaço ou '\n'

        if (word.empty() && !isBreak) continue;

        if (!current.empty() && MeasureText((current + " " + word).c_str(), fontSize) <= maxWidth) {
            current += " " + word;
        } else if (!current.empty() && MeasureText(word.c_str(), fontSize) <= maxWidth) {
            lines.push_back(current);
            current = word;
        } else if (MeasureText(word.c_str(), fontSize) <= maxWidth) {
            current = word;
        } else {
            // Palavra maior que a linha: quebra caractere a caractere.
            if (!current.empty()) {
                lines.push_back(current);
                current.clear();
            }
            std::string chunk;
            for (char c : word) {
                if (!chunk.empty() && MeasureText((chunk + c).c_str(), fontSize) > maxWidth) {
                    lines.push_back(chunk);
                    chunk.clear();
                }
                chunk += c;
            }
            current = chunk;
        }

        if (isBreak) {
            lines.push_back(current);
            current.clear();
        }
    }

    if (!current.empty()) lines.push_back(current);
    if (lines.empty()) lines.push_back("");
    return lines;
}

} // namespace util
