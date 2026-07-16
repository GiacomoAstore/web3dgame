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

*(vuoto — popolare man mano che si scrive il codice)*

---

## Engine — Renderer (WebGL2)

*(vuoto — qui andranno le funzioni di init contesto GL, gestione shader, buffer, camera, draw call)*

---

## Engine — Input

*(vuoto — gestione tastiera/mouse/gamepad via SDL2 o Emscripten HTML5 API)*

---

## Engine — Memoria / Risorse

*(vuoto — allocator custom, caricamento asset, pool di oggetti)*

---

## Game — Logica di gioco

*(vuoto — entità, stato di gioco, regole)*

---

## Interoperabilità JS ↔ Wasm

> Ogni funzione che attraversa il confine C++ ↔ JS va qui, obbligatoriamente (vedi `REGOLE.md §5`).

*(vuoto)*

---

## Networking — Multiplayer P2P

> Architettura: P2P assistito via WebRTC DataChannel, modello **host-authoritative**, stanze 2-8 giocatori. Vedi `REGOLE.md §9` per i vincoli obbligatori.

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
- Vedi `emsdk-version.txt` alla root del progetto.

### Comando build standard
```
emcmake cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

---

## Log delle decisioni architetturali (ADR leggero)

> Quando si prende una decisione strutturale (es. "usiamo un ECS", "il game loop gira a timestep fisso"), annotarla qui con data e motivazione breve. Non serve un formato ADR completo, bastano 2-3 righe.

- **AAAA-MM-GG** — *(esempio)* Deciso di usare WebGL2 invece di WebGPU: supporto browser più ampio e maturo con Emscripten al momento della scelta.
- **2026-07-16** — Stack confermato: C/C++17 + Emscripten, rendering WebGL2.
- **2026-07-16** — Genere scelto: **racing arcade**, stanze da 2 a 8 giocatori. Genere adatto al P2P perché lo stato di gioco (posizione/velocità veicoli) è compatto e tollerante a piccoli errori di sincronizzazione rispetto a un genere con combattimento preciso.
- **2026-07-16** — Architettura di rete: **P2P assistito** via WebRTC DataChannel, modello **host-authoritative** (un peer per stanza è autorità sullo stato di gara). Scelto invece di un vero P2P simmetrico per evitare la complessità/vulnerabilità di sincronizzare fisica autorevole su più peer contemporaneamente. Richiede comunque un piccolo servizio di signaling esterno per lo scambio iniziale SDP/ICE (il browser non permette socket P2P grezzi).
