#!/usr/bin/env bash
set -e

DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" >/dev/null 2>&1 && pwd)"
cd "$DIR"

echo "=========================================================="
echo "  Smart Academic Recommendation System (SARS) Runner"
echo "=========================================================="

# 1. Compile C++17 Core Engine if binary is missing
if [ ! -f "cpp_engine/bin/sars_engine" ]; then
  echo "==> Compiling C++17 Engine..."
  cd cpp_engine && ./build.sh && cd ..
fi

# 2. Check Node dependencies
if [ ! -d "server/node_modules" ]; then
  echo "==> Installing Node.js server dependencies..."
  cd server && npm install && cd ..
fi

# 3. Launch Server
echo "==> Launching SARS on http://localhost:3000..."
node server/server.js
