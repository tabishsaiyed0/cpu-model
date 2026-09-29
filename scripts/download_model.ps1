# Download a tiny CPU-friendly GGUF model (~0.8GB, ~20-40 tok/s on laptop CPU)
# Run: powershell -ExecutionPolicy Bypass -File scripts/download_model.ps1
$ErrorActionPreference = "Stop"
$dir = "models"
New-Item -ItemType Directory -Force -Path $dir | Out-Null

# Default: Llama 3.2 1B Instruct Q4_K_M — best quality/speed balance for CPU in 2026
# Alternative (faster, weaker): SmolLM2-360M-Instruct Q4_K_M (~200MB)
$MODEL_URL = "https://huggingface.co/bartowski/Llama-3.2-1B-Instruct-GGUF/resolve/main/Llama-3.2-1B-Instruct-Q4_K_M.gguf"
$OUT = "$dir/dm-1b-q4_k_m.gguf"

if (Test-Path $OUT) { Write-Host "Already exists: $OUT"; exit 0 }
Write-Host "Downloading $MODEL_URL ..."
Invoke-WebRequest -Uri $MODEL_URL -OutFile $OUT -UseBasicParsing
Write-Host "Done: $OUT"
