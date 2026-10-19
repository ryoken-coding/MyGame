#include "core/SaveSystem.h"

#include <filesystem>

#include "util/Files.h"
#include "util/Json.h"
#include "util/Strings.h"

using util::Json;

const char* SaveSystem::defaultPath()
{
    return "saves/savegame.json";
}

bool SaveSystem::save(const SaveData& data, std::string& error, const std::string& path)
{
    Json root = Json::makeObject();
    root["version"] = Json(1);
    root["area"] = Json(data.area);

    Json player = Json::makeObject();
    player["x"] = Json((double)data.playerX);
    player["y"] = Json((double)data.playerY);
    player["hp"] = Json(data.hp);
    player["maxHp"] = Json(data.maxHp);
    root["player"] = std::move(player);

    Json inventory = Json::makeObject();
    inventory["potions"] = Json(data.potions);
    inventory["coins"] = Json(data.coins);
    inventory["key"] = Json(data.key);
    inventory["flower"] = Json(data.flower);
    root["inventory"] = std::move(inventory);

    Json quest = Json::makeObject();
    quest["state"] = Json(questStateName(data.quest.state));
    quest["bossDefeated"] = Json(data.quest.bossDefeated);
    quest["flowerTaken"] = Json(data.quest.flowerTaken);
    root["quest"] = std::move(quest);

    Json flags = Json::makeArray();
    for (const std::string& flag : data.worldFlags) {
        flags.push_back(Json(flag));
    }
    root["worldFlags"] = std::move(flags);

    return util::writeTextFile(path, root.serialize(2), error);
}

bool SaveSystem::load(SaveData& out, std::string& error, const std::string& path)
{
    std::string content;
    if (!util::readTextFile(path, content, error)) {
        error = util::formatText("Falha ao carregar o save '%s': %s", path.c_str(), error.c_str());
        return false;
    }

    Json root;
    if (!Json::parse(content, root, error)) {
        error = util::formatText("Save '%s' corrompido ou inválido. %s", path.c_str(), error.c_str());
        return false;
    }

    if (!root.isObject() || root["version"].asInt() != 1) {
        error = util::formatText("Save '%s' tem formato desconhecido (esperado version = 1).", path.c_str());
        return false;
    }

    out = SaveData{};
    out.area = root["area"].asString("vila");
    out.playerX = (float)root["player"]["x"].asDouble();
    out.playerY = (float)root["player"]["y"].asDouble();
    out.hp = root["player"]["hp"].asInt(100);
    out.maxHp = root["player"]["maxHp"].asInt(100);

    out.potions = root["inventory"]["potions"].asInt(0);
    out.coins = root["inventory"]["coins"].asInt(0);
    out.key = root["inventory"]["key"].asBool(false);
    out.flower = root["inventory"]["flower"].asBool(false);

    out.quest.state = questStateFromName(root["quest"]["state"].asString("not_started"));
    out.quest.bossDefeated = root["quest"]["bossDefeated"].asBool(false);
    out.quest.flowerTaken = root["quest"]["flowerTaken"].asBool(false);

    out.worldFlags.clear();
    for (const Json& flag : root["worldFlags"].asArray()) {
        if (flag.isString()) out.worldFlags.push_back(flag.asString());
    }

    // Sanidade básica: valores negativos ou área vazia indicam save inválido.
    if (out.hp <= 0 || out.maxHp <= 0 || out.area.empty()) {
        error = util::formatText("Save '%s' contém valores inválidos (vida ou área).", path.c_str());
        return false;
    }
    return true;
}

bool SaveSystem::exists(const std::string& path)
{
    std::error_code ec;
    return std::filesystem::exists(path, ec);
}
