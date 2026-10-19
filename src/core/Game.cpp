#include "core/Game.h"

#include <cmath>

#include "raylib.h"
#include "systems/Combat.h"
#include "ui/Hud.h"
#include "util/Strings.h"
#include "world/Areas.h"

namespace {

// Distância máxima para interagir com NPCs.
constexpr float kInteractRange = 56.0f;
constexpr int kPotionHealAmount = 35;

Vector2 normalize(Vector2 v)
{
    const float len = std::sqrt(v.x * v.x + v.y * v.y);
    if (len < 0.0001f) return {0.0f, 0.0f};
    return {v.x / len, v.y / len};
}

float clampf(float value, float lo, float hi)
{
    return value < lo ? lo : (value > hi ? hi : value);
}

} // namespace

bool Game::init(std::string& error)
{
    error.clear();

    world_.setAreaDefs(buildAreaDefs());

    if (!sounds_.init()) {
        // Áudio indisponível não impede o jogo (comum em ambientes sem som).
        TraceLog(LOG_WARNING, "Dispositivo de áudio indisponível: o jogo seguirá sem efeitos sonoros.");
    }

    camera_.offset = Vector2{GetScreenWidth() * 0.5f, GetScreenHeight() * 0.5f};
    camera_.target = camera_.offset;
    camera_.rotation = 0.0f;
    camera_.zoom = 1.0f;
    return true;
}

void Game::shutdown()
{
    sounds_.shutdown();
}

// ---------------------------------------------------------------- estados

void Game::handleTitleInput()
{
    const int itemCount = 3;
    const bool canLoad = SaveSystem().exists();

    if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) {
        titleSelection_ = (titleSelection_ + 1) % itemCount;
        sounds_.play(Sfx::MenuMove);
    }
    if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) {
        titleSelection_ = (titleSelection_ + itemCount - 1) % itemCount;
        sounds_.play(Sfx::MenuMove);
    }

    if (IsKeyPressed(KEY_ESCAPE)) {
        wantsQuit_ = true;
        return;
    }

    if (!IsKeyPressed(KEY_ENTER) && !IsKeyPressed(KEY_SPACE)) return;

    switch (titleSelection_) {
    case 0: {
        std::string err;
        if (startNewGame(err)) {
            sounds_.play(Sfx::Confirm);
        } else {
            notice_ = err;
            sounds_.play(Sfx::Cancel);
        }
        break;
    }
    case 1: {
        if (!canLoad) {
            sounds_.play(Sfx::Cancel);
            notice_ = "Nenhum jogo salvo encontrado em 'saves/savegame.json'.";
            break;
        }
        std::string err;
        if (loadGame(err)) {
            sounds_.play(Sfx::Confirm);
        } else {
            notice_ = err;
            sounds_.play(Sfx::Cancel);
        }
        break;
    }
    case 2:
        wantsQuit_ = true;
        break;
    default:
        break;
    }
}

void Game::handlePlayingInput(float dt)
{
    if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_P)) {
        if (dialogue_.active()) {
            dialogue_.cancel();
            dialogueOutcome_ = DialogueOutcome::None;
        } else {
            state_ = GameState::Paused;
            pauseSelection_ = 0;
            sounds_.play(Sfx::Cancel);
        }
        return;
    }

    // Durante o diálogo, só o avance de páginas é aceito.
    if (dialogue_.active()) {
        if (IsKeyPressed(KEY_E) || IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
            dialogue_.advance();
            if (!dialogue_.active()) {
                sounds_.play(Sfx::Confirm);
                if (dialogueOutcome_ == DialogueOutcome::AcceptQuest) {
                    quest_.state = QuestState::Active;
                    messages_.add("Nova missão: A Flor de Lumina!");
                    sounds_.play(Sfx::Quest);
                } else if (dialogueOutcome_ == DialogueOutcome::CompleteQuest) {
                    quest_.state = QuestState::Completed;
                    player_.inventory.add(ItemType::Coin, 50);
                    player_.inventory.add(ItemType::Potion, 2);
                    messages_.add("Missão concluída! Recompensa: 50 moedas e 2 poções.");
                    onQuestCompleted();
                }
                dialogueOutcome_ = DialogueOutcome::None;
            } else {
                sounds_.play(Sfx::MenuMove);
            }
        }
        return;
    }

    // Movimento em quatro direções.
    Vector2 moveDir{0.0f, 0.0f};
    if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)) moveDir.y -= 1.0f;
    if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) moveDir.y += 1.0f;
    if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) moveDir.x -= 1.0f;
    if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) moveDir.x += 1.0f;

    if (moveDir.x != 0.0f || moveDir.y != 0.0f) {
        player_.applyMove(dt, world_.map(), normalize(moveDir));
    }
    player_.updateTimers(dt);

    if (IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_J)) doAttack();
    if (IsKeyPressed(KEY_E) || IsKeyPressed(KEY_ENTER)) tryInteract();

    if (IsKeyPressed(KEY_Q) || IsKeyPressed(KEY_ONE)) {
        if (player_.inventory.remove(ItemType::Potion, 1)) {
            player_.heal(kPotionHealAmount);
            sounds_.play(Sfx::Potion);
            messages_.add(util::formatText("Você usou uma Poção de Vida (+%d).", kPotionHealAmount));
        } else {
            messages_.add("Você não tem poções.");
            sounds_.play(Sfx::Cancel);
        }
    }
}

void Game::handlePauseInput()
{
    const std::vector<std::string> items = {
        "Continuar", "Salvar Jogo", "Carregar Jogo", "Reiniciar Partida", "Sair para o Título"};
    const int count = (int)items.size();

    if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) {
        pauseSelection_ = (pauseSelection_ + 1) % count;
        sounds_.play(Sfx::MenuMove);
    }
    if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) {
        pauseSelection_ = (pauseSelection_ + count - 1) % count;
        sounds_.play(Sfx::MenuMove);
    }

    if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_P)) {
        state_ = GameState::Playing;
        sounds_.play(Sfx::Confirm);
        return;
    }

    if (!IsKeyPressed(KEY_ENTER) && !IsKeyPressed(KEY_SPACE)) return;

    std::string err;
    switch (pauseSelection_) {
    case 0:
        state_ = GameState::Playing;
        sounds_.play(Sfx::Confirm);
        break;
    case 1:
        if (saveGame(err)) {
            sounds_.play(Sfx::Save);
            messages_.add("Jogo salvo em 'saves/savegame.json'.");
        } else {
            sounds_.play(Sfx::Cancel);
            messages_.add(err);
        }
        break;
    case 2:
        if (!SaveSystem().exists()) {
            sounds_.play(Sfx::Cancel);
            messages_.add("Nenhum jogo salvo encontrado.");
        } else if (loadGame(err)) {
            sounds_.play(Sfx::Confirm);
            messages_.add("Jogo carregado.");
        } else {
            sounds_.play(Sfx::Cancel);
            messages_.add(err);
        }
        break;
    case 3:
        if (startNewGame(err)) {
            sounds_.play(Sfx::Confirm);
            messages_.add("Partida reiniciada.");
        }
        break;
    case 4:
        state_ = GameState::Title;
        titleSelection_ = 0;
        sounds_.play(Sfx::Cancel);
        break;
    default:
        break;
    }
}

void Game::handleGameOverInput()
{
    const int count = 2;
    if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S) || IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) {
        gameOverSelection_ = (gameOverSelection_ + 1) % count;
        sounds_.play(Sfx::MenuMove);
    }
    if (IsKeyPressed(KEY_ESCAPE)) {
        state_ = GameState::Title;
        return;
    }
    if (!IsKeyPressed(KEY_ENTER) && !IsKeyPressed(KEY_SPACE)) return;

    std::string err;
    if (gameOverSelection_ == 0) {
        if (startNewGame(err)) sounds_.play(Sfx::Confirm);
    } else {
        state_ = GameState::Title;
        titleSelection_ = 0;
        sounds_.play(Sfx::Cancel);
    }
}

void Game::handleVictoryInput()
{
    const int count = 2;
    if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S) || IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) {
        victorySelection_ = (victorySelection_ + 1) % count;
        sounds_.play(Sfx::MenuMove);
    }
    if (IsKeyPressed(KEY_ESCAPE)) {
        state_ = GameState::Title;
        return;
    }
    if (!IsKeyPressed(KEY_ENTER) && !IsKeyPressed(KEY_SPACE)) return;

    std::string err;
    if (victorySelection_ == 0) {
        if (startNewGame(err)) sounds_.play(Sfx::Confirm);
    } else {
        state_ = GameState::Title;
        titleSelection_ = 0;
        sounds_.play(Sfx::Cancel);
    }
}

// ------------------------------------------------------------------ update

void Game::update(float dt)
{
    if (dt > 0.05f) dt = 0.05f; // evita saltos após travamentos

    messages_.update(dt);
    if (gateHintCooldown_ > 0.0f) gateHintCooldown_ -= dt;

    switch (state_) {
    case GameState::Title:
        handleTitleInput();
        break;

    case GameState::Playing:
        handlePlayingInput(dt);
        if (state_ == GameState::Playing && player_.alive) {
            world_.update(dt, player_.center());
            checkPickups();
            checkEnemyContact();
            if (state_ == GameState::Playing && player_.alive) {
                checkGate();
                checkTransitions();
            }
        }
        updateCamera(dt);

        // Dica de interação quando há um NPC por perto.
        pendingInteract_.clear();
        if (!dialogue_.active() && state_ == GameState::Playing) {
            for (const Npc& npc : world_.npcs()) {
                const Vector2 d = Vector2{npc.center().x - player_.center().x,
                                          npc.center().y - player_.center().y};
                if (std::sqrt(d.x * d.x + d.y * d.y) <= kInteractRange) {
                    pendingInteract_ = util::formatText("[E] Falar com %s", npc.displayName());
                    break;
                }
            }
        }
        break;

    case GameState::Paused:
        handlePauseInput();
        break;

    case GameState::GameOver:
        handleGameOverInput();
        break;

    case GameState::Victory:
        handleVictoryInput();
        break;
    }
}

void Game::updateCamera(float dt)
{
    const Vector2 target = player_.center();
    const float lerp = clampf(6.0f * dt, 0.0f, 1.0f);
    camera_.target.x += (target.x - camera_.target.x) * lerp;
    camera_.target.y += (target.y - camera_.target.y) * lerp;
    camera_.offset = Vector2{GetScreenWidth() * 0.5f, GetScreenHeight() * 0.5f};

    // Mantém a visão dentro dos limites do mapa (quando ele é maior que a tela).
    const float halfW = GetScreenWidth() * 0.5f / camera_.zoom;
    const float halfH = GetScreenHeight() * 0.5f / camera_.zoom;
    const float mapW = world_.map().widthPixels();
    const float mapH = world_.map().heightPixels();

    if (mapW > GetScreenWidth()) {
        camera_.target.x = clampf(camera_.target.x, halfW, mapW - halfW);
    } else {
        camera_.target.x = mapW * 0.5f;
    }
    if (mapH > GetScreenHeight()) {
        camera_.target.y = clampf(camera_.target.y, halfH, mapH - halfH);
    } else {
        camera_.target.y = mapH * 0.5f;
    }
}

// ---------------------------------------------------------------- gameplay

void Game::tryInteract()
{
    for (const Npc& npc : world_.npcs()) {
        const Vector2 d = Vector2{npc.center().x - player_.center().x,
                                  npc.center().y - player_.center().y};
        if (std::sqrt(d.x * d.x + d.y * d.y) > kInteractRange) continue;

        dialogueOutcome_ = DialogueOutcome::None;
        if (npc.kind() == NpcKind::Elder) {
            if (quest_.state == QuestState::NotStarted) {
                dialogueOutcome_ = DialogueOutcome::AcceptQuest;
            } else if (quest_.state == QuestState::ReturnWithFlower) {
                dialogueOutcome_ = DialogueOutcome::CompleteQuest;
            }
        }

        dialogue_.start(buildNpcDialogue(npc.kind(), quest_));
        sounds_.play(Sfx::Confirm);
        return;
    }
}

void Game::doAttack()
{
    combat::resolvePlayerAttack(player_, world_, quest_, messages_, sounds_, enemiesDefeated_);
}

void Game::checkPickups()
{
    std::vector<ItemPickup>& pickups = world_.pickups();
    for (size_t i = 0; i < pickups.size();) {
        ItemPickup& pickup = pickups[i];
        if (!CheckCollisionRecs(player_.bounds(), pickup.bounds())) {
            i++;
            continue;
        }

        const ItemType type = pickup.type();
        player_.inventory.add(type, pickup.amount());
        world_.setFlag(pickup.id());

        switch (type) {
        case ItemType::Potion:
            sounds_.play(Sfx::Potion);
            messages_.add("Poção de Vida obtida! Use com [Q].");
            break;
        case ItemType::Coin:
            sounds_.play(Sfx::Coin);
            messages_.add(util::formatText("+%d moedas.", pickup.amount()));
            break;
        case ItemType::Key:
            sounds_.play(Sfx::Key);
            messages_.add("Chave Antiga obtida! Ela abre o portão ao norte da floresta.");
            break;
        case ItemType::Flower:
            sounds_.play(Sfx::Quest);
            quest_.flowerTaken = true;
            quest_.state = QuestState::ReturnWithFlower;
            messages_.add("Flor de Lumina recuperada! Volte com o Ancião.");
            break;
        default:
            break;
        }

        pickups.erase(pickups.begin() + static_cast<long>(i));
    }
}

void Game::checkEnemyContact()
{
    if (combat::resolveEnemyContact(player_, world_, sounds_)) {
        onPlayerDeath();
    }
}

void Game::checkGate()
{
    if (world_.map().gatesOpen()) return;

    // Procura tiles de portão próximos ao jogador.
    Rectangle probe = player_.bounds();
    probe.x -= 10.0f;
    probe.y -= 10.0f;
    probe.width += 20.0f;
    probe.height += 20.0f;

    const int ts = TileMap::kTileSize;
    const int x0 = (int)std::floor(probe.x / ts);
    const int y0 = (int)std::floor(probe.y / ts);
    const int x1 = (int)std::floor((probe.x + probe.width) / ts);
    const int y1 = (int)std::floor((probe.y + probe.height) / ts);

    for (int ty = y0; ty <= y1; ty++) {
        for (int tx = x0; tx <= x1; tx++) {
            if (world_.map().at(tx, ty) != Tile::Gate) continue;

            if (player_.inventory.has(ItemType::Key)) {
                world_.map().setGatesOpen(true);
                world_.setFlag(World::kGateFlag);
                sounds_.play(Sfx::Gate);
                messages_.add("Você usou a Chave Antiga. O portão de pedra se abre!");
            } else if (gateHintCooldown_ <= 0.0f) {
                messages_.add("O portão está trancado. Você precisa da Chave Antiga.");
                gateHintCooldown_ = 3.0f;
            }
            return;
        }
    }
}

void Game::checkTransitions()
{
    const TransitionZone* zone = world_.transitionAt(player_.bounds());
    if (zone == nullptr) return;

    std::string err;
    if (!changeArea(zone->targetArea, zone->targetSpawn, err)) {
        messages_.add(err);
        sounds_.play(Sfx::Cancel);
        return;
    }

    if (!zone->message.empty()) messages_.add(zone->message);
    sounds_.play(Sfx::Confirm);

    // Salvamento automático ao trocar de área.
    if (!saveGame(err)) {
        messages_.add(util::formatText("Autossalvamento falhou: %s", err.c_str()));
    }
}

// -------------------------------------------------------------- transições

bool Game::changeArea(const std::string& areaName, const std::string& spawnName, std::string& error)
{
    if (!world_.loadArea(areaName, error)) return false;
    world_.spawnQuestDropIfNeeded(quest_.bossDefeated, quest_.flowerTaken);
    placePlayerAt(spawnName);
    return true;
}

void Game::placePlayerAt(const std::string& spawnName)
{
    const Vector2 spawn = world_.spawnPosition(spawnName);
    player_.position = Vector2{spawn.x - player_.size.x * 0.5f, spawn.y - player_.size.y * 0.5f};
    camera_.target = player_.center();
}

void Game::onPlayerDeath()
{
    sounds_.play(Sfx::EnemyDie);
    messages_.add("Você foi derrotado...");
    state_ = GameState::GameOver;
    gameOverSelection_ = 0;
}

void Game::onQuestCompleted()
{
    sounds_.play(Sfx::Quest);
    state_ = GameState::Victory;
    victorySelection_ = 0;
}

// -------------------------------------------------------------- persistência

bool Game::startNewGame(std::string& error)
{
    error.clear();
    player_.reset();
    quest_ = QuestStatus{};
    enemiesDefeated_ = 0;
    dialogue_.cancel();
    dialogueOutcome_ = DialogueOutcome::None;
    world_.worldFlags().clear();
    messages_.clear();

    if (!world_.loadArea("vila", error)) return false;
    world_.spawnQuestDropIfNeeded(quest_.bossDefeated, quest_.flowerTaken);
    placePlayerAt("start");

    messages_.add("Bem-vindo a Eldara: A Flor de Lumina!");
    messages_.add("Fale com o Ancião na praça para receber sua missão.");
    state_ = GameState::Playing;
    return true;
}

SaveData Game::collectSaveData() const
{
    SaveData data;
    data.area = world_.areaName();
    data.playerX = player_.position.x;
    data.playerY = player_.position.y;
    data.hp = player_.hp;
    data.maxHp = player_.maxHp;
    data.potions = player_.inventory.count(ItemType::Potion);
    data.coins = player_.inventory.count(ItemType::Coin);
    data.key = player_.inventory.has(ItemType::Key);
    data.flower = player_.inventory.has(ItemType::Flower);
    data.quest = quest_;
    data.worldFlags.assign(world_.worldFlags().begin(), world_.worldFlags().end());
    return data;
}

void Game::applySaveData(const SaveData& data)
{
    player_.reset();

    quest_ = data.quest;
    player_.hp = data.hp;
    player_.maxHp = data.maxHp;
    player_.inventory.add(ItemType::Potion, data.potions);
    player_.inventory.add(ItemType::Coin, data.coins);
    if (data.key) player_.inventory.add(ItemType::Key, 1);
    if (data.flower) player_.inventory.add(ItemType::Flower, 1);

    // Flags antes do carregamento da área: entidades removidas não reaparecem.
    world_.worldFlags().clear();
    for (const std::string& flag : data.worldFlags) world_.setFlag(flag);

    std::string error;
    if (!world_.loadArea(data.area, error)) {
        // Se a área do save falhar, volta para a vila para o jogo não quebrar.
        TraceLog(LOG_ERROR, "%s", error.c_str());
        messages_.add(error);
        world_.loadArea("vila", error);
        placePlayerAt("start");
        return;
    }

    world_.spawnQuestDropIfNeeded(quest_.bossDefeated, quest_.flowerTaken);
    player_.position = Vector2{data.playerX, data.playerY};
    camera_.target = player_.center();
}

bool Game::saveGame(std::string& error)
{
    SaveSystem saver;
    return saver.save(collectSaveData(), error);
}

bool Game::loadGame(std::string& error)
{
    SaveData data;
    SaveSystem saver;
    if (!saver.load(data, error)) return false;

    applySaveData(data);
    messages_.clear();
    messages_.add("Jogo carregado com sucesso.");
    state_ = GameState::Playing;
    return true;
}

// -------------------------------------------------------------------- draw

void Game::drawWorld(float time) const
{
    BeginMode2D(camera_);
    world_.draw(time, player_.center());
    player_.draw(time);
    EndMode2D();
}

void Game::draw(float time)
{
    switch (state_) {
    case GameState::Title: {
        const std::vector<std::string> items = {"Novo Jogo", "Carregar Jogo", "Sair"};
        ui::drawTitleScreen(items, titleSelection_, notice_);
        break;
    }

    case GameState::Playing:
        ClearBackground(Color{20, 20, 30, 255});
        drawWorld(time);
        ui::drawHud(player_, quest_);
        if (!pendingInteract_.empty()) ui::drawInteractionHint(pendingInteract_);
        ui::drawDialogueBox(dialogue_);
        messages_.draw(16, 130);
        break;

    case GameState::Paused:
        ClearBackground(Color{20, 20, 30, 255});
        drawWorld(time);
        ui::drawHud(player_, quest_);
        messages_.draw(16, 130);
        ui::drawPauseOverlay({"Continuar", "Salvar Jogo", "Carregar Jogo", "Reiniciar Partida",
                              "Sair para o Título"},
                             pauseSelection_);
        break;

    case GameState::GameOver:
        ClearBackground(Color{20, 20, 30, 255});
        drawWorld(time);
        ui::drawGameOverScreen({"Reiniciar Partida", "Sair para o Título"}, gameOverSelection_);
        messages_.draw(16, 130);
        break;

    case GameState::Victory:
        ClearBackground(Color{20, 20, 30, 255});
        drawWorld(time);
        ui::drawVictoryScreen({"Jogar Novamente", "Sair para o Título"}, victorySelection_,
                              player_.inventory.count(ItemType::Coin), enemiesDefeated_);
        messages_.draw(16, 130);
        break;
    }
}
