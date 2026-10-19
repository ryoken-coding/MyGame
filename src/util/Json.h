// Json.h - JSON mínimo (leitura/escrita) usado pelo sistema de salvamento.
// Suporta: null, bool, número, string, array e objeto, com mensagens de erro claras.
#pragma once

#include <map>
#include <string>
#include <variant>
#include <vector>

namespace util {

class Json {
public:
    enum class Type { Null, Bool, Number, String, Array, Object };

    using Array = std::vector<Json>;
    using Object = std::map<std::string, Json>;

    Json() : data_(nullptr) {}
    explicit Json(bool value) : data_(value) {}
    explicit Json(double value) : data_(value) {}
    explicit Json(int value) : data_(static_cast<double>(value)) {}
    explicit Json(const char* value) : data_(std::string(value)) {}
    explicit Json(std::string value) : data_(std::move(value)) {}
    explicit Json(Array value) : data_(std::move(value)) {}
    explicit Json(Object value) : data_(std::move(value)) {}

    static Json makeArray() { return Json(Array{}); }
    static Json makeObject() { return Json(Object{}); }

    Type type() const;
    bool isNull() const { return type() == Type::Null; }
    bool isBool() const { return type() == Type::Bool; }
    bool isNumber() const { return type() == Type::Number; }
    bool isString() const { return type() == Type::String; }
    bool isArray() const { return type() == Type::Array; }
    bool isObject() const { return type() == Type::Object; }

    // Acessores com valor padrão (não lançam exceções).
    bool asBool(bool fallback = false) const;
    double asDouble(double fallback = 0.0) const;
    int asInt(int fallback = 0) const;
    std::string asString(const std::string& fallback = "") const;

    const Array& asArray() const;
    const Object& asObject() const;
    Array& asArray();   // converte para array vazio se necessário
    Object& asObject(); // converte para objeto vazio se necessário

    bool contains(const std::string& key) const;
    // Retorna um Json nulo estático se a chave não existir.
    const Json& operator[](const std::string& key) const;
    // Cria a chave se necessário (para escrita).
    Json& operator[](const std::string& key);

    void push_back(Json value);

    int size() const;

    std::string serialize(int indent = 2) const;
    static bool parse(const std::string& text, Json& out, std::string& error);

private:
    std::variant<std::nullptr_t, bool, double, std::string, Array, Object> data_;

    void serializeTo(std::string& out, int indent, int depth) const;
};

} // namespace util
