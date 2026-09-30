# dm-cli — CPU-only Dungeon Master (llama.cpp)

A terminal RPG where a tiny quantized LLM (no GPU) DMs your adventure.
C++ client + `llama-server` in CPU mode (`-ngl 0`) + GBNF grammar for valid JSON game state.

## About
- 100% offline, CPU-only runs on a laptop / cheap VPS.
- Grammar-constrained generation: model *must* output `{"narration","location","hp_change","loot"}`.
- Tiny + fast: 1B Q4_K_M ≈ 0.8GB RAM, ~10-30 tok/s on 4-core CPU. No API keys.

## Quickstart (Windows, 5 min)

1. Build llama.cpp once:
```powershell
git clone https://github.com/ggml-org/llama.cpp ..\llama.cpp
cmake -S ..\llama.cpp -B ..\llama.cpp\build -DCMAKE_BUILD_TYPE=Release
cmake --build ..\llama.cpp\build --config Release
```

2. Download model + run server (CPU only):
```powershell
powershell -ExecutionPolicy Bypass -File scripts/download_model.ps1
powershell -ExecutionPolicy Bypass -File scripts/run_server.ps1
# leaves http://127.0.0.1:8080 running
```

3. Build + play (new terminal):
```powershell
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
.\build\Release\dm-cli.exe
# or: .\build\dm-cli.exe (MinGW/Ninja layout)
```

## How it works
- `src/main.cpp` — game loop, HP/location state, POSTs to `/completion`.
- `grammars/game-state.gbnf` — enforced server-side via `--grammar-file`, guarantees parseable JSON.
- `prompts/system.txt` — DM personality + rules. Edit to make sci-fi / horror / pirate variants.
- `scripts/` — model download + CPU server launcher.

## Try next (make it yours)
- Swap model: SmolLM2-360M Q4_K_M for toasters, Qwen2.5-1.5B Q4_K_M for smarter DM.
- Add `llama-bench` run to README with your tok/s.
- Save `history` to `save.json` and add `--load` flag.
- Add speculative draft model (`-md ...1B...`) for +40% CPU speed.
