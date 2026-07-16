# REGOLE FERREE — Da leggere PRIMA di scrivere qualsiasi codice

> Stack: C/C++17 + Emscripten + WebGL2 (SDL2 o GLFW per finestra/input)
> Questo file è vincolante per agenti AI e sviluppatori umani. Non è un suggerimento: è un contratto.

---

## 0. Regola zero: documentazione prima del codice

1. Prima di scrivere una riga di codice, **leggi `DOCUMENTAZIONE.md`** per intero.
2. Cerca se la funzione/modulo che ti serve esiste già. **Non duplicare mai** una funzione esistente.
3. Se crei, rinomini, sposti o elimini una funzione/file, **aggiorna `DOCUMENTAZIONE.md` nello stesso commit**. Un commit che tocca il codice senza toccare la documentazione, quando serve, è considerato incompleto e va corretto.
4. Se non sai dove mettere qualcosa, fermati e chiedi — non improvvisare una nuova struttura di cartelle.

---

## 1. Setup ambiente (bloccante)

- Versione Emscripten **pinnata** in `emsdk-version.txt` — mai usare `latest` in CI o build di release.
- Build system: **CMake** (`CMakeLists.txt` alla root + uno per modulo se serve). Niente Makefile scritti a mano.
- Ogni build deve poter essere riprodotta con:
  ```
  emcmake cmake -B build -DCMAKE_BUILD_TYPE=Release
  cmake --build build
  ```
- Nessun percorso assoluto hardcoded (niente `/home/utente/...`) in nessun file di build o codice.

---

## 2. Standard di codice C/C++

- Standard: **C++17** minimo. Se serve C puro per moduli specifici, isolarlo e motivarlo in `DOCUMENTAZIONE.md`.
- **Vietati i raw pointer per ownership.** Usa `std::unique_ptr` / `std::shared_ptr`. Raw pointer solo per riferimenti non proprietari (parametri "osserva ma non possiedi").
- **Vietato `new`/`delete` manuale** fuori da allocator custom esplicitamente documentati.
- **Vietate le allocazioni dinamiche nel game loop / hot path** (niente `malloc`, `new`, `std::vector::push_back` non pre-riservato dentro il render/update loop). Pre-allocare tutto all'inizializzazione.
- Naming: `PascalCase` per classi/struct, `camelCase` per funzioni e variabili, `SCREAMING_SNAKE_CASE` per costanti/macro.
- Ogni file `.h`/`.hpp` deve avere `#pragma once`.
- Nessun `using namespace std;` globale nei file header.
- Warning trattati come errori in build di CI (`-Wall -Wextra -Werror`).

---

## 3. Architettura

- Separazione netta e non negoziabile tra:
  - **Engine** (rendering, input, audio, memoria, math) — non conosce le regole del gioco.
  - **Game logic** (entità, regole, stato di gioco) — usa l'engine, non lo modifica.
- Comunicazione tra moduli solo attraverso interfacce dichiarate negli header pubblici del modulo. Nessun accesso diretto a variabili interne di un altro modulo.
- Ogni nuovo modulo richiede una entry in `DOCUMENTAZIONE.md` **prima** di essere integrato nel resto del progetto.

---

## 4. Regole WebGL2 / rendering

- Controllare `glGetError()` dopo ogni chiamata critica in fase di sviluppo (dietro flag `DEBUG_GL`).
- Usare sempre **VAO** per la configurazione dei vertex attribute, mai stato globale implicito.
- Minimizzare i cambi di stato GL (state thrashing): raggruppare le draw call per shader/materiale.
- Nessuna allocazione di buffer GPU dentro il loop di rendering: creare i buffer in fase di init/load.

---

## 5. Confine C++ ↔ JavaScript (Emscripten)

- Minimizzare le chiamate attraverso il boundary Wasm↔JS: ogni chiamata ha un costo, non chiamare funzioni JS per singolo oggetto/frame se puoi raggruppare i dati e passarli in batch.
- Ogni funzione esposta con `EMSCRIPTEN_BINDINGS` o `EM_JS`/`EM_ASM` deve essere documentata in `DOCUMENTAZIONE.md` nella sezione "Interoperabilità JS".
- Nessun dato passato come stringa JSON attraverso il boundary se esiste un'alternativa con memoria condivisa (`HEAPU8`, puntatori, ecc.) per dati ad alta frequenza (es. ogni frame).

---

## 6. Gestione memoria (critica in Wasm)

- Ogni risorsa allocata (texture, buffer, mesh, suoni) deve avere un owner chiaro e un percorso di distruzione esplicito documentato.
- Vietati i memory leak "accettati temporaneamente": se una risorsa non viene liberata, è un bug, non un TODO.
- Build di debug periodiche con Emscripten `-sASSERTIONS=1` e sanitizer (`-fsanitize=address`, dove supportato) prima di ogni merge importante.

---

## 7. Testing e verifica prima del commit

- Nessun commit va fatto senza aver **compilato con successo** in locale (`emcmake` + `cmake --build`).
- Nessun commit su feature di rendering/input va fatto senza aver **testato nel browser** (Chrome e Firefox almeno una volta).
- Se il modulo ha logica non banale (fisica, gioco, parsing), scrivere almeno un test minimo prima di considerarlo "fatto".

---

## 8. Git e commit

- Un commit = una modifica logica. Niente commit "varie ed eventuali".
- Messaggio di commit: `[modulo] descrizione breve` (es. `[renderer] aggiunge frustum culling`).
- Se il commit tocca la documentazione: indicarlo nel messaggio o in un commit separato immediatamente successivo.

---

## 9. Cosa fare se una regola sembra bloccarti

- Non aggirarla in silenzio. Segnalarla esplicitamente (commento `// REGOLA-ECCEZIONE: motivo` nel codice + nota in `DOCUMENTAZIONE.md`) e proporre l'eccezione prima di procedere.
