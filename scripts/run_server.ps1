# Run llama-server in CPU-only mode with grammar enforcement
# Usage: powershell -ExecutionPolicy Bypass -File scripts/run_server.ps1
# Prereqs: build llama.cpp (https://github.com/ggml-org/llama.cpp) OR place llama-server.exe on PATH
$ErrorActionPreference = "Stop"
$MODEL = "models/dm-1b-q4_k_m.gguf"
$GRAMMAR = "grammars/game-state.gbnf"

if (!(Test-Path $MODEL)) { Write-Error "Missing $MODEL. Run scripts/download_model.ps1 first."; exit 1 }

$SERVER = Get-Command llama-server -ErrorAction SilentlyContinue
if ($SERVER) { $BIN = "llama-server" }
elseif (Test-Path "..\llama.cpp\build\bin\Release\llama-server.exe") { $BIN = "..\llama.cpp\build\bin\Release\llama-server.exe" }
elseif (Test-Path "..\llama.cpp\build\bin\llama-server.exe") { $BIN = "..\llama.cpp\build\bin\llama-server.exe" }
else { Write-Error "llama-server not found. Clone + build llama.cpp first (see README)."; exit 1 }

# -ngl 0 = CPU only, -t = threads, -c = context
& $BIN -m $MODEL --grammar-file $GRAMMAR -ngl 0 -t 4 -c 4096 --port 8080
