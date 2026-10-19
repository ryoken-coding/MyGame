#include "util/Json.h"

#include <cctype>
#include <cmath>
#include <cstdio>
#include <cstring>

#include "util/Strings.h"

namespace util {

namespace {

const Json& nullJson()
{
    static const Json value;
    return value;
}

void appendEscaped(std::string& out, const std::string& s)
{
    out += '"';
    for (unsigned char c : s) {
        switch (c) {
        case '"': out += "\\\""; break;
        case '\\': out += "\\\\"; break;
        case '\n': out += "\\n"; break;
        case '\r': out += "\\r"; break;
        case '\t': out += "\\t"; break;
        default:
            if (c < 0x20) {
                out += formatText("\\u%04x", c);
            } else {
                out += static_cast<char>(c); // bytes UTF-8 passam direto
            }
        }
    }
    out += '"';
}

// Analisador recursivo-descendente com posição para mensagens de erro.
class Parser {
public:
    Parser(const std::string& text, std::string& error) : text_(text), error_(error) {}

    bool parse(Json& out)
    {
        skipWhitespace();
        if (!parseValue(out)) return false;
        skipWhitespace();
        if (pos_ != text_.size()) return fail("conteúdo inesperado após o valor");
        return true;
    }

private:
    const std::string& text_;
    std::string& error_;
    size_t pos_ = 0;

    bool fail(const std::string& message)
    {
        // Conta a linha/coluna para facilitar a depuração de saves corrompidos.
        size_t line = 1, col = 1;
        for (size_t i = 0; i < pos_ && i < text_.size(); i++) {
            if (text_[i] == '\n') { line++; col = 1; } else { col++; }
        }
        error_ = formatText("JSON inválido: %s (linha %zu, coluna %zu)", message.c_str(), line, col);
        return false;
    }

    void skipWhitespace()
    {
        while (pos_ < text_.size() && std::isspace(static_cast<unsigned char>(text_[pos_]))) pos_++;
    }

    bool peek(char c) const { return pos_ < text_.size() && text_[pos_] == c; }

    bool parseValue(Json& out)
    {
        if (pos_ >= text_.size()) return fail("fim inesperado do arquivo");
        const char c = text_[pos_];
        if (c == '{') return parseObject(out);
        if (c == '[') return parseArray(out);
        if (c == '"') {
            std::string s;
            if (!parseString(s)) return false;
            out = Json(std::move(s));
            return true;
        }
        if (c == 't') return parseLiteral("true", Json(true), out);
        if (c == 'f') return parseLiteral("false", Json(false), out);
        if (c == 'n') return parseLiteral("null", Json(), out);
        if (c == '-' || std::isdigit(static_cast<unsigned char>(c))) return parseNumber(out);
        return fail(formatText("caractere inesperado '%c'", c));
    }

    bool parseLiteral(const char* literal, Json value, Json& out)
    {
        const size_t len = std::strlen(literal);
        if (text_.compare(pos_, len, literal) != 0) return fail(formatText("esperava '%s'", literal));
        pos_ += len;
        out = std::move(value);
        return true;
    }

    bool parseNumber(Json& out)
    {
        const size_t start = pos_;
        if (peek('-')) pos_++;
        while (pos_ < text_.size() && std::isdigit(static_cast<unsigned char>(text_[pos_]))) pos_++;
        if (peek('.')) {
            pos_++;
            while (pos_ < text_.size() && std::isdigit(static_cast<unsigned char>(text_[pos_]))) pos_++;
        }
        if (pos_ < text_.size() && (text_[pos_] == 'e' || text_[pos_] == 'E')) {
            pos_++;
            if (peek('+') || peek('-')) pos_++;
            while (pos_ < text_.size() && std::isdigit(static_cast<unsigned char>(text_[pos_]))) pos_++;
        }
        if (pos_ == start) return fail("número inválido");
        out = Json(std::stod(text_.substr(start, pos_ - start)));
        return true;
    }

    bool parseString(std::string& out)
    {
        if (!peek('"')) return fail("esperava uma string");
        pos_++;
        out.clear();
        while (true) {
            if (pos_ >= text_.size()) return fail("string não terminada");
            const char c = text_[pos_++];
            if (c == '"') return true;
            if (c == '\\') {
                if (pos_ >= text_.size()) return fail("escape incompleto");
                const char e = text_[pos_++];
                switch (e) {
                case '"': out += '"'; break;
                case '\\': out += '\\'; break;
                case '/': out += '/'; break;
                case 'n': out += '\n'; break;
                case 'r': out += '\r'; break;
                case 't': out += '\t'; break;
                case 'b': out += '\b'; break;
                case 'f': out += '\f'; break;
                case 'u': {
                    if (pos_ + 4 > text_.size()) return fail("escape \\u incompleto");
                    unsigned int code = 0;
                    for (int i = 0; i < 4; i++) {
                        const char h = text_[pos_++];
                        code <<= 4;
                        if (h >= '0' && h <= '9') code |= static_cast<unsigned int>(h - '0');
                        else if (h >= 'a' && h <= 'f') code |= static_cast<unsigned int>(h - 'a' + 10);
                        else if (h >= 'A' && h <= 'F') code |= static_cast<unsigned int>(h - 'A' + 10);
                        else return fail("escape \\u inválido");
                    }
                    // Converte o codepoint para UTF-8.
                    if (code < 0x80) {
                        out += static_cast<char>(code);
                    } else if (code < 0x800) {
                        out += static_cast<char>(0xC0 | (code >> 6));
                        out += static_cast<char>(0x80 | (code & 0x3F));
                    } else {
                        out += static_cast<char>(0xE0 | (code >> 12));
                        out += static_cast<char>(0x80 | ((code >> 6) & 0x3F));
                        out += static_cast<char>(0x80 | (code & 0x3F));
                    }
                    break;
                }
                default: return fail("escape desconhecido");
                }
            } else {
                out += c;
            }
        }
    }

    bool parseArray(Json& out)
    {
        pos_++; // '['
        Json::Array items;
        skipWhitespace();
        if (peek(']')) { pos_++; out = Json(std::move(items)); return true; }
        while (true) {
            Json value;
            skipWhitespace();
            if (!parseValue(value)) return false;
            items.push_back(std::move(value));
            skipWhitespace();
            if (peek(',')) { pos_++; continue; }
            if (peek(']')) { pos_++; out = Json(std::move(items)); return true; }
            return fail("esperava ',' ou ']' no array");
        }
    }

    bool parseObject(Json& out)
    {
        pos_++; // '{'
        Json::Object fields;
        skipWhitespace();
        if (peek('}')) { pos_++; out = Json(std::move(fields)); return true; }
        while (true) {
            skipWhitespace();
            std::string key;
            if (!parseString(key)) return false;
            skipWhitespace();
            if (!peek(':')) return fail("esperava ':' após a chave");
            pos_++;
            skipWhitespace();
            Json value;
            if (!parseValue(value)) return false;
            fields[key] = std::move(value);
            skipWhitespace();
            if (peek(',')) { pos_++; continue; }
            if (peek('}')) { pos_++; out = Json(std::move(fields)); return true; }
            return fail("esperava ',' ou '}' no objeto");
        }
    }
};

} // namespace

Json::Type Json::type() const
{
    switch (data_.index()) {
    case 1: return Type::Bool;
    case 2: return Type::Number;
    case 3: return Type::String;
    case 4: return Type::Array;
    case 5: return Type::Object;
    default: return Type::Null;
    }
}

bool Json::asBool(bool fallback) const
{
    if (const auto* v = std::get_if<bool>(&data_)) return *v;
    return fallback;
}

double Json::asDouble(double fallback) const
{
    if (const auto* v = std::get_if<double>(&data_)) return *v;
    return fallback;
}

int Json::asInt(int fallback) const
{
    if (const auto* v = std::get_if<double>(&data_)) return static_cast<int>(*v);
    return fallback;
}

std::string Json::asString(const std::string& fallback) const
{
    if (const auto* v = std::get_if<std::string>(&data_)) return *v;
    return fallback;
}

const Json::Array& Json::asArray() const
{
    static const Array empty;
    if (const auto* v = std::get_if<Array>(&data_)) return *v;
    return empty;
}

const Json::Object& Json::asObject() const
{
    static const Object empty;
    if (const auto* v = std::get_if<Object>(&data_)) return *v;
    return empty;
}

Json::Array& Json::asArray()
{
    if (!std::holds_alternative<Array>(data_)) data_ = Array{};
    return std::get<Array>(data_);
}

Json::Object& Json::asObject()
{
    if (!std::holds_alternative<Object>(data_)) data_ = Object{};
    return std::get<Object>(data_);
}

bool Json::contains(const std::string& key) const
{
    if (const auto* v = std::get_if<Object>(&data_)) return v->find(key) != v->end();
    return false;
}

const Json& Json::operator[](const std::string& key) const
{
    if (const auto* v = std::get_if<Object>(&data_)) {
        auto it = v->find(key);
        if (it != v->end()) return it->second;
    }
    return nullJson();
}

Json& Json::operator[](const std::string& key)
{
    return asObject()[key];
}

void Json::push_back(Json value)
{
    asArray().push_back(std::move(value));
}

int Json::size() const
{
    if (const auto* v = std::get_if<Array>(&data_)) return static_cast<int>(v->size());
    if (const auto* v = std::get_if<Object>(&data_)) return static_cast<int>(v->size());
    return 0;
}

void Json::serializeTo(std::string& out, int indent, int depth) const
{
    const std::string pad(static_cast<size_t>(indent * (depth + 1)), ' ');
    const std::string padEnd(static_cast<size_t>(indent * depth), ' ');

    switch (type()) {
    case Type::Null: out += "null"; break;
    case Type::Bool: out += asBool() ? "true" : "false"; break;
    case Type::Number: {
        const double n = asDouble();
        if (std::floor(n) == n && std::abs(n) < 1e15) {
            out += formatText("%lld", static_cast<long long>(n));
        } else {
            out += formatText("%.6g", n);
        }
        break;
    }
    case Type::String: appendEscaped(out, asString()); break;
    case Type::Array: {
        const Array& items = asArray();
        if (items.empty()) { out += "[]"; break; }
        out += "[\n";
        for (size_t i = 0; i < items.size(); i++) {
            out += pad;
            items[i].serializeTo(out, indent, depth + 1);
            if (i + 1 < items.size()) out += ",";
            out += "\n";
        }
        out += padEnd + "]";
        break;
    }
    case Type::Object: {
        const Object& fields = asObject();
        if (fields.empty()) { out += "{}"; break; }
        out += "{\n";
        size_t i = 0;
        for (const auto& [key, value] : fields) {
            out += pad;
            appendEscaped(out, key);
            out += ": ";
            value.serializeTo(out, indent, depth + 1);
            if (++i < fields.size()) out += ",";
            out += "\n";
        }
        out += padEnd + "}";
        break;
    }
    }
}

std::string Json::serialize(int indent) const
{
    std::string out;
    serializeTo(out, indent, 0);
    out += "\n";
    return out;
}

bool Json::parse(const std::string& text, Json& out, std::string& error)
{
    error.clear();
    Parser parser(text, error);
    return parser.parse(out);
}

} // namespace util
