# DOCUMENTAZIONE VIVENTE — Da leggere prima di ogni modifica, da aggiornare ad ogni commit

> Questo file è la fonte di verità sullo stato reale del codice.
> Regola: se il codice cambia e questo file no, la modifica è considerata incompleta (vedi `REGOLE.md §0`).
> Non descrivere qui cosa il codice *dovrebbe* fare: descrivi cosa fa **davvero**, oggi.

---

## Come usare questo file

1. **Prima di scrivere codice**: cerca qui (Ctrl+F sul nome funzione/modulo) se qualcosa di simile esiste già.
2. **Dopo aver scritto codice**: aggiungi/aggiorna la voce corrispondente nella sezione giusta, nello stesso commit.
3. **Se elimini qualcosa**: rimuovi la voce, non lasciarla come "deprecato" a meno che non sia intenzionale e motivato.
4. Ogni voce segue questo formato fisso:

```
### nomeFunzione(parametri) -> tipoRitorno
- File: percorso/relativo/al/file.cpp
- Modulo: (es. Renderer / Input / Physics / Game / JS-Interop)
- Descrizione: cosa fa, in linguaggio semplice, in 1-3 frasi.
- Parametri: elenco con tipo e significato
- Ritorna: cosa restituisce e in quali condizioni può fallire
- Dipendenze: da cosa dipende / cosa chiama internamente
- Chiamata da: dove viene usata (facoltativo ma consigliato)
- Ultimo aggiornamento: AAAA-MM-GG
```

---

## Indice moduli

- [Engine — Core / Bootstrap](#engine--core--bootstrap)
- [Engine — Renderer (WebGL2)](#engine--renderer-webgl2)
- [Engine — Input](#engine--input)
- [Engine — Memoria / Risorse](#engine--memoria--risorse)
- [Game — Logica di gioco](#game--logica-di-gioco)
- [Interoperabilità JS ↔ Wasm](#interoperabilità-js--wasm)
- [Networking — Multiplayer P2P](#networking--multiplayer-p2p)
- [Build / Tooling](#build--tooling)

---

## Engine — Core / Bootstrap

### Engine
- **File**: `src/engine/core/engine.hpp`, `src/engine/core/engine.cpp`
- **Modulo**: Engine::Core
- **Descrizione**: Classe `Engine` che rappresenta il core del motore di gioco. Gestisce l'inizializzazione del renderer e del game logic, il game loop principale, e il timing dei frame. Espone il singleton globale `g_engine` per l'accesso da Emscripten.
- **Responsabilità**:
  - Inizializzazione e shutdown di tutti i sottosistemi (Renderer, Game)
  - Main loop orchestration: `Tick()` chiama Game::Update + Renderer::BeginFrame + Game::Render + Renderer::EndFrame
  - Frame timing e delta time calculation
  - Controllo dello stato di esecuzione (IsRunning, RequestShutdown)
- **Dipendenze**: Renderer, Game
- **Entry point Emscripten**: `main()` in `src/main.cpp` crea `g_engine`, chiama `Init()`, e lancia il loop via `emscripten_set_main_loop()`
- **Ultimo aggiornamento**: 2026-07-16

---

## Engine — Renderer (WebGL2)

### Renderer
- **File**: `src/engine/renderer/renderer.hpp`, `src/engine/renderer/renderer.cpp`
- **Modulo**: Engine::Renderer
- **Descrizione**: Classe `Renderer` che gestisce il contesto WebGL2, creazione shader, VAO/VBO, draw call e caricamento asset 3D. Non conosce la logica di gioco; riceve istruzioni da Game tramite `DrawMesh()`.
- **API principali**:
  - `Init(width, height)`: Inizializza contesto GL, clear color, depth test, shader default, e carica il modello `vehicle-truck-yellow.glb`
  - `BeginFrame() / EndFrame()`: Clear framebuffer e flush draw call
  - `LoadModel(modelPath)`: Carica un file glTF/glb, bufferizza i vertici e gli indici, gestisce immagini texture esterne via `stb_image`
  - `DrawMesh(meshId, shader, mvpMatrix)`: Esegue una draw call con VAO + shader + MVP matrix, supporta mappe texture esterne quando presenti
  - `Shutdown()`: Libera GPU resource (VAO, VBO, EBO, shader programs, texture)
- **Shader default**: Vertex/fragment in GLSL ES 300 (WebGL2), prende position (vec3) + texcoord (vec2), usa texture se presente oppure bianco uniforme
- **Asset 3D importati**:
  - `assets/models/vehicle-truck-yellow.glb`: veicolo di test
  - `assets/models/track-straight.glb`: pezzo pista lineare
  - `assets/models/track-corner.glb`: pezzo pista curvo
  - `assets/models/colormap.png`: texture esterna condivisa (non embedded)
- **Provenienza**: Kenney Starter-Kit-Racing, GitHub KenneyNL/Starter-Kit-Racing
- **Licenza**: CC0 (licenza inclusa in `assets/models/LICENSE`)
- **State management**: 
  - VAO vincolato per tutta la durata della draw call per evitare state thrashing
  - Depth test enabled (GL_LEQUAL)
  - Nessuna allocazione dinamica durante render (tutto pre-allocato in Init)
- **Debug**: Flag `DEBUG_GL` abilita `glGetError()` checks dopo ogni call critica
- **Ultimo aggiornamento**: 2026-07-16

### Shader (classe helper)
- **File**: `src/engine/renderer/renderer.hpp/.cpp`
- **Descrizione**: Wrapper minimalista attorno a un OpenGL program handle. Permette `Use()` e `SetUniformMatrix4()`.
- **Ultimo aggiornamento**: 2026-07-16

### Math utilities (header-only)
- **File**: `src/engine/math/math.hpp`
- **Modulo**: Engine::Math
- **Descrizione**: Aggregazione di GLM (header-only). Espone typedef `Vec2/3/4`, `Mat4`, `Quat`, e namespace `Math::` con funzioni helper (Perspective, LookAt, Translate, Rotate, Scale, Normalize, Cross, Dot, ecc.)
- **Note**: GLM è assunto disponibile. La build usa prima `find_package(glm)` e se non trova il package usa la copia vendorizzata in `third_party/glm`.
- **Ultimo aggiornamento**: 2026-07-16

---

## Engine — Input

*(Placeholder: non ancora implementato. Input gestito da Emscripten HTML5 API quando arriverà il turno.)*

---

## Engine — Memoria / Risorse

*(Placeholder: non ancora implementato. Allocator custom e pool di risorse andranno qui quando servono asset complexi.)*

---

## Game — Logica di gioco

### Game
- **File**: `src/game/game.hpp`, `src/game/game.cpp`
- **Modulo**: Game::Application
- **Descrizione**: Classe `Game` che rappresenta la logica applicativa del gioco. Non conosce i dettagli di rendering; riceve un puntatore `Renderer*` solo nei metodi `Init()` e `Render()`.
- **API**:
  - `Init(renderer)`: Inizializzazione game state (placeholder: solo log). Chiamato da Engine dopo Renderer::Init.
  - `Update(deltaTime)`: Aggiornamento logica (fisica, input, entità) ogni frame. Pre-update prima di Render.
  - `Render(renderer)`: Rendering: calcola camera (perspective projection, view matrix), applica rotazione al triangolo test, chiama `renderer->DrawMesh()`.
- **State attuale**:
  - Semplice triangolo rotante per test di rendering
  - Prospettiva fissa: camera a (0, 0, 2) che guarda l'origine
  - Rotazione lenta attorno asse Y basata su tempo trascorso
- **TODO**: Entità, fisica, input, controllo veicoli
- **Ultimo aggiornamento**: 2026-07-16

---

## Interoperabilità JS ↔ Wasm

> Ogni funzione che attraversa il confine C++ ↔ JS va qui, obbligatoriamente (vedi `REGOLE.md §5`).

*(Placeholder: fase Bootstrap. Attualmente nessuna funzione esportata a JS via EMSCRIPTEN_BINDINGS. Emscripten invoca C++ solo tramite `emscripten_set_main_loop()`.)*

**Future bindings** (da aggiungere quando necessario):
- Input handler per gestire eventi JavaScript (click, keydown, ecc.) dal browser verso C++
- Connessione WebRTC signaling da JS hacia C++

---

## Networking — Multiplayer P2P

> Architettura: P2P assistito via WebRTC DataChannel, modello **host-authoritative**, stanze 2-8 giocatori. Vedi `REGOLE.md §9` per i vincoli obbligatori.

### Stato attuale

- **File**: `src/net/networking.hpp`, `src/net/networking.cpp`
- **Stato**: PLACEHOLDER - Non implementato
- **Commento in code**: `// PLACEHOLDER - Networking not yet implemented (see REGOLE.md §9)`
- **Classe stub**: `NetworkManager` con metodi vuoti `Init()`, `Shutdown()`, `Update()`

### Componenti previsti
- **Client (Wasm/browser)**: gioca, e se è l'host, calcola anche la fisica autorevole della stanza.
- **Servizio di signaling** (fuori dal binario di gioco, es. piccolo server Node.js/WebSocket): scambia SDP/ICE tra i peer per aprire il DataChannel. Non tocca la logica di gioco.
- **STUN** (pubblico) per NAT traversal; **TURN** di fallback da prevedere (non ancora implementato).

### Formato messaggi di rete
*(da compilare mano a mano che si implementano — ogni messaggio va documentato qui prima di essere scritto in codice, vedi regola §9)*

```
### NomeMessaggio
- Direzione: host->client / client->host
- Canale: reliable / unreliable
- Frequenza: (es. 20 Hz, on-event)
- Campi: elenco campo:tipo:significato
- Dimensione stimata: byte
```

### Stato migrazione host
*(da definire: rielezione vs terminazione gara — documentare qui la strategia scelta prima di implementarla)*

---

## Build / Tooling

### Versione Emscripten in uso
- File: `emsdk-version.txt`
- Versione: **3.1.57**

### Struttura cartelle del progetto
```
/workspaces/web3dgame/
├── CMakeLists.txt                      # Build root (Emscripten)
├── emsdk-version.txt                   # Versione pinnata
├── .gitignore
├── REGOLE.md
├── DOCUMENTAZIONE.md
├── build/                              # (build output, gitignore)
│
├── src/
│   ├── engine/
│   │   ├── CMakeLists.txt              # Build modulo engine
│   │   ├── core/
│   │   │   ├── engine.hpp
│   │   │   └── engine.cpp
│   │   ├── renderer/
│   │   │   ├── renderer.hpp
│   │   │   └── renderer.cpp
│   │   └── math/
│   │       └── math.hpp                # Header-only GLM utilities
│   │
│   ├── game/
│   │   ├── CMakeLists.txt              # Build modulo game
│   │   ├── game.hpp
│   │   └── game.cpp
│   │
│   ├── net/
│   │   ├── CMakeLists.txt              # Build modulo net
│   │   ├── networking.hpp
│   │   └── networking.cpp              # PLACEHOLDER
│   │
│   └── main.cpp                        # Entry point Emscripten (crea g_engine, lancia main loop)
│
├── assets/                             # (vuoto per ora, pronto per shader/modelli)
│
├── web/
│   ├── index.html                      # HTML landing page (con loading screen)
│   ├── shell.html                      # Template Emscripten (wrapper minimalista)
│   └── (web3dgame.html/js generati da build)
│
└── tests/                              # (vuoto per scaffolding, pronto per unit tests)
```

### Comando build standard (Emscripten)
```bash
emcmake cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

**Output**: 
- `build/web3dgame.html` (page + inline JS)
- `build/web3dgame.wasm` (modulo WebAssembly)

### Dipendenze esterne (dev)
- **Emscripten 3.1.57** (compilatore, CMake toolchain)
- **GLM** (header-only, math library — assumo disponibile in `/usr/include` o package manager)
- **GLEW** (opzionale: non usato su Emscripten, linkato solo su desktop)

### Flags CMake
- **C++17** standard
- **Warnings as errors**: `-Wall -Wextra -Werror`
- **Emscripten specifici**:
  - `-s USE_SDL=2`: SDL2 per window/input management (non utilizzato ancora, ma linkato per futuro)
  - `-s USE_WEBGL2=1`: WebGL2 support
  - `-s WASM=1`: Produce WebAssembly binaries
  - `-s ALLOW_MEMORY_GROWTH=1`: Consenti crescita memoria (utile per caricare asset)
  - `-s NO_EXIT_RUNTIME=0`: Permetti exit da Emscripten main loop

### Debug build
```bash
emcmake cmake -B build -DCMAKE_BUILD_TYPE=Debug -DDEBUG_GL=ON
cmake --build build
```

Ultimo aggiornamento: 2026-07-16

---

## Log delle decisioni architetturali (ADR leggero)

> Quando si prende una decisione strutturale (es. "usiamo un ECS", "il game loop gira a timestep fisso"), annotarla qui con data e motivazione breve. Non serve un formato ADR completo, bastano 2-3 righe.

- **2026-07-16** — Stack confermato: C/C++17 + Emscripten 3.1.57, rendering WebGL2.
- **2026-07-16** — Genere scelto: **racing arcade**, stanze da 2 a 8 giocatori. Genere adatto al P2P perché lo stato di gioco (posizione/velocità veicoli) è compatto e tollerante a piccoli errori di sincronizzazione rispetto a un genere con combattimento preciso.
- **2026-07-16** — Architettura di rete: **P2P assistito** via WebRTC DataChannel, modello **host-authoritative** (un peer per stanza è autorità sullo stato di gara). Scelto invece di un vero P2P simmetrico per evitare la complessità/vulnerabilità di sincronizzare fisica autorevole su più peer contemporaneamente. Richiede comunque un piccolo servizio di signaling esterno per lo scambio iniziale SDP/ICE (il browser non permette socket P2P grezzi).
- **2026-07-16** — **SDL2 vs GLFW**: Scelto **SDL2**. Motivazione: profilo Emscripten migliore (traduzione diretta di input HTML5 via Emscripten API built-in), supporto audio nativo (servirà per SFX di gioco), documentazione e esempi più maturi con Emscripten. GLFW ha meno supporto specifico per input/events HTML5 in Emscripten. (Nota: SDL2 è linkato a livello CMake ma non utilizzato nel codice C++ attuale — servirà per input in fase successiva.)
- **2026-07-16** — **Struttura moduli**: Engine (renderer + core) separato nettamente da Game logic (regola §3 REGOLE.md). Math utilities come header-only GLM wrapper. Networking in modulo separato (placeholder finché non implementato). Ogni modulo ha CMakeLists.txt e è linkato staticamente all'executable principale.
- **2026-07-16** — **Game loop timing**: Frame timing basato su `std::chrono` in Emscripten, con cap a 50ms per evitare jump di delta time (es. quando tab perde focus). Main loop su `emscripten_set_main_loop()` con target ~60 fps (requestAnimationFrame).
- **2026-07-16** — **Shader in build time**: Shader GLSL ES 300 compilati runtime in C++ (non asset separati per ora). Quando arriveranno shader complessi, verranno spostati in file separati (`assets/shaders/*.glsl`) e caricati con allocator dedicato.
