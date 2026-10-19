# Eldara: A Flor de Lumina

Jogo 2D de RPG para um jogador, feito em **C++20** com a biblioteca
[raylib](https://www.raylib.com/) (gráficos, áudio e entrada) e **CMake**.

Em uma pequena aventura de fantasia top-down, você explora a Vila de Pedra Clara,
a Floresta Sombria e a Caverna do Guardião. Converse com os moradores, enfrente
monstros, colete itens e recupere a **Flor de Lumina** — o coração sagrado do bosque.

Toda a arte é desenhada em tempo de execução com formas simples (retângulos,
círculos e polígonos) e todos os efeitos sonoros são gerados proceduralmente.
Não há arquivos de arte ou áudio externos para baixar.

## Características

- Movimentação em quatro direções com colisão contra o cenário.
- Três áreas conectadas por transições (vila → floresta → caverna).
- Combate simples: ataque com espada, inimigos que perseguem o jogador,
  pontos de vida, dano por contato, vitória e derrota.
- Quatro tipos de inimigo (Slime, Goblin, Goblin Líder e o chefe Guardião).
- Inventário com itens coletáveis: **Poção de Vida** (cura), moedas,
  **Chave Antiga** (abre o portão da caverna) e a Flor de Lumina.
- Diálogos com NPCs e uma missão completa com objetivo e recompensa.
- Interface com barra de vida, inventário, rastreador da missão e mensagens.
- Tela inicial, pausa, derrota (com reinício) e tela de vitória.
- Salvamento local em JSON (`saves/savegame.json`), com salvamento automático
  ao trocar de área e opção manual no menu de pausa.

## Requisitos

- Compilador com suporte a **C++20** (GCC 10+, Clang 12+, MSVC 2019 16.11+).
- **CMake 3.22** ou superior.
- **Git** (usado pelo CMake para baixar o raylib automaticamente, caso ele
  não esteja instalado no sistema).
- Em Linux, as dependências de desenvolvimento do raylib: janela e OpenGL
  (veja abaixo).

## Compilação e execução

Os comandos são os mesmos nos três sistemas. O CMake procura um raylib
instalado no sistema e, se não encontrar, baixa a versão 6.2 automaticamente
do repositório oficial durante a configuração (é preciso ter internet na
primeira compilação).

```bash
# na raiz do projeto
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

Para executar, use a raiz do projeto como diretório de trabalho (os mapas
ficam em `assets/maps/`):

```bash
# Linux / macOS
./build/mygame

# Windows (PowerShell, build com Visual Studio)
.\build\Release\mygame.exe
```

> Os recursos também são copiados para a pasta do executável após a compilação,
> então o jogo também funciona se for iniciado a partir de `build/` (ou
> `build/Release/` no Windows).

### Linux (dependências)

```bash
# Debian / Ubuntu
sudo apt install build-essential cmake git libgl1-mesa-dev libx11-dev \
    libxrandr-dev libxinerama-dev libxcursor-dev libxi-dev libasound2-dev

# Fedora
sudo dnf install gcc-c++ cmake git mesa-libGL-devel libX11-devel \
    libXrandr-devel libXinerama-devel libXcursor-devel libXi-devel alsa-lib-devel
```

### macOS

```bash
xcode-select --install   # command line tools (clang, make)
brew install cmake git   # se ainda não tiver
```

### Windows

- Instale o [Visual Studio](https://visualstudio.microsoft.com/) com a carga de
  trabalho *Desenvolvimento em C++*, ou o MinGW-w64.
- Instale o [CMake](https://cmake.org/download/) e o
  [Git](https://git-scm.com/download/win).
- No prompt do "x64 Native Tools Command Prompt for VS" (ou no PowerShell com o
  CMake no `PATH`), use os mesmos comandos da seção anterior.
- Alternativa com vcpkg: `vcpkg install raylib` e depois
  `cmake -B build -DCMAKE_TOOLCHAIN_FILE=<vcpkg>/scripts/buildsystems/vcpkg.cmake`.

## Controles

| Ação                       | Teclas                     |
| -------------------------- | -------------------------- |
| Mover                      | `WASD` ou setas            |
| Atacar                     | `Espaço` ou `J`            |
| Interagir / avançar diálogo | `E` ou `Enter`            |
| Usar Poção de Vida         | `Q` ou `1`                 |
| Pausar / voltar            | `Esc` ou `P`               |
| Confirmar nos menus        | `Enter` ou `Espaço`        |

## Como jogar

1. Você começa na praça da **Vila de Pedra Clara**. Fale com o **Ancião**
   (tecle `E`) para receber a missão *A Flor de Lumina*.
2. Siga para leste e entre na **Floresta Sombria**. Derrote monstros, colete
   poções e moedas e procure o **Goblin Líder** — ele carrega a **Chave Antiga**
   do portão ao norte.
3. Com a chave, abra o portão e adentre a **Caverna do Guardião**. Derrote o
   Guardião (cuidado: ele fica mais rápido quando ferido) e recupere a flor.
4. Volte com a **Flor de Lumina** até o Ancião para concluir a missão e ver a
   tela de vitória.

Dicas: use poções com `Q` quando a vida estiver baixa; o salvamento automático
ocorre ao mudar de área, e você pode salvar/carregar manualmente no menu de pausa
(`Esc`). Se for derrotado, dá para reiniciar a partida pela própria tela.

## Salvamento

O progresso fica em `saves/savegame.json` (criado automaticamente), em formato
JSON legível: posição e vida do jogador, inventário, estado da missão e as
"flags" do mundo (inimigos já derrotados, itens coletados, portão aberto...).
Apagar o arquivo reinicia o progresso. Erros de leitura/escrita são informados
na própria tela, com mensagens explicando o que falhou.

## Estrutura do projeto

```
MyGame/
├── CMakeLists.txt          # configuração do build (baixa o raylib se necessário)
├── README.md
├── assets/
│   └── maps/               # mapas em ASCII (legenda abaixo)
│       ├── vila.txt
│       ├── floresta.txt
│       └── caverna.txt
└── src/
    ├── main.cpp            # janela, laço principal e modo --smoke
    ├── core/               # Game (máquina de estados) e SaveSystem (JSON)
    ├── entities/           # Player, Enemy, Npc, ItemPickup, Entity
    ├── world/              # TileMap (colisão/desenho), Areas, World
    ├── systems/            # Combat, Inventory, Dialogue, Quest
    ├── ui/                 # Hud (vida, menus, diálogo) e MessageLog
    ├── audio/              # SoundBank (sons gerados proceduralmente)
    └── util/               # Json, Strings, Files (caminhos e erros)
```

### Legenda dos mapas

Os mapas em `assets/maps/` são grades de caracteres (`.txt`, versionados).
Cada linha é uma fileira de tiles; todos os marcadores de spawn ficam sobre
grama:

| Caractere | Significado               | Caractere | Significado              |
| --------- | ------------------------- | --------- | ------------------------ |
| `.`       | grama (chão)              | `P`       | spawn do jogador         |
| `,`       | caminho de terra          | `n`       | Ancião (NPC)             |
| `~`       | água (sólida)             | `m`       | Aldeão (NPC)             |
| `T`       | árvore (sólida)           | `s`       | Slime                    |
| `R`       | rocha (sólida)            | `g`       | Goblin                   |
| `#`       | parede/alvenaria (sólida) | `k`       | Goblin Líder (carrega a chave) |
| `F`       | flores/cristais           | `b`       | Guardião (chefe)         |
| `X`       | portão (abre com a chave) | `h`       | Poção de Vida            |
|           |                           | `c`       | Moedas                   |

## Verificação automatizada

O jogo possui um modo de autoverificação, útil em CI ou em máquinas sem janela:

```bash
./build/mygame --smoke 120
```

Ele inicia uma partida, roda 120 quadros, grava e relê o save e retorna código
de saída 0 se tudo ocorrer bem (mensagens detalhadas saem no terminal).

## Solução de problemas

- **"Arquivo de mapa não encontrado"** — execute o jogo a partir da raiz do
  do projeto (onde fica `assets/`), ou a partir da pasta do executável (os
  recursos são copiados para lá no build).
- **O CMake não encontra o raylib** — na primeira configuração o CMake baixa o
  raylib automaticamente; é preciso estar conectado à internet e ter o `git`
  instalado.
- **Sem áudio** — o jogo avisa no terminal e continua sem efeitos sonoros.
