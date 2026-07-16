# Configurazione Emscripten per Web3D Game

## Problema Attuale
Il progetto necessita di **Emscripten 3.1.57** per compilare il codice C++ a WebAssembly.

## Soluzioni

### Opzione 1: Installazione Manuale su Windows (Consigliato)

1. **Scarica Emscripten SDK Portable per Windows:**
   ```bash
   # Scarica il file ZIP da GitHub
   # https://github.com/emscripten-core/emsdk/releases
   # Cerca: emsdk-3.1.57-windows-x64.zip
   
   # Oppure scarica da riga di comando:
   curl -L -o emsdk.zip "https://github.com/emscripten-core/emsdk/archive/main.zip"
   ```

2. **Estrai la cartella:**
   ```bash
   Expand-Archive emsdk.zip -DestinationPath .
   cd emsdk-main
   ```

3. **Configura Emscripten:**
   ```bash
   .\emsdk install 3.1.57
   .\emsdk activate 3.1.57
   ```

4. **Attiva l'ambiente (ogni volta prima di compilare):**
   ```bash
   .\emsdk_env.bat
   ```

5. **Compila il progetto:**
   ```bash
   cd web3dgame
   mkdir build
   cd build
   emcmake cmake ..
   emmake make
   ```

### Opzione 2: Usare WSL2 + Ubuntu (Alternativa)

Dopo il riavvio del sistema (WSL è già stato installato):

```bash
# In PowerShell
wsl -d Ubuntu

# Nel terminale Ubuntu
sudo apt-get update
sudo apt-get install -y git

cd /mnt/c/Users/user/Desktop/workspace/web3dgame
git clone https://github.com/emscripten-core/emsdk.git
cd emsdk
./emsdk install 3.1.57
./emsdk activate 3.1.57
source ./emsdk_env.sh

cd ..
mkdir -p build
cd build
emcmake cmake ..
emmake make
```

### Opzione 3: Usare Docker

Se hai Docker installato:

```bash
docker run --rm -v "C:/Users/user/Desktop/workspace/web3dgame:/workspace" -w /workspace emscripten/emsdk:3.1.57 bash -c "
  mkdir -p build && cd build && \
  emcmake cmake .. && \
  emmake make
"
```

## Output Atteso

Una volta compilato correttamente, i seguenti file saranno generati in `web/`:
- `web3dgame.js` - Wrapper JavaScript generato da Emscripten
- `web3dgame.wasm` - Binary WebAssembly compilato
- `web3dgame.wasm.map` - Source map per debugging

## Verifica della Compilazione

Dopo la compilazione, il progetto dovrebbe caricarsi correttamente all'indirizzo:
```
http://localhost:8000/index.html
```

## Troubleshooting

**Se `emcmake` o `emmake` non vengono trovati:**
- Assicurati di aver eseguito `emsdk activate 3.1.57`
- Su Windows, esegui `emsdk_env.bat` prima di compilare
- Su Linux/WSL, esegui `source emsdk_env.sh`

**Se CMake non trova GLM:**
```bash
# Installa GLM (su Ubuntu)
sudo apt-get install libglm-dev

# O su Windows con vcpkg
vcpkg install glm
```

---

**Status Attuale:** Server web è in esecuzione su `http://localhost:8000`, ma la compilazione WASM è necessaria per il funzionamento completo.
