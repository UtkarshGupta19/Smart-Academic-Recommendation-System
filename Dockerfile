# ==========================================================
# SARS // Smart Academic Recommendation System
# Production Multi-Stage Dockerfile for Cloud Deployment
# ==========================================================

# ----------------------------------------------------------
# Stage 1: Build & Verify C++17 Core Algorithmic Engine
# ----------------------------------------------------------
FROM gcc:12 AS cpp-builder
WORKDIR /build

# Copy C++ source, headers, Makefile, and automated tests
COPY cpp_engine/ ./cpp_engine/

# Compile engine and execute automated unit tests during build
RUN cd cpp_engine && CXX=g++ make all && ./bin/test_suite

# ----------------------------------------------------------
# Stage 2: Lightweight Production Node.js Runtime
# ----------------------------------------------------------
FROM node:20-slim
WORKDIR /app

# Install standard C++ runtime libraries required by native binary
RUN apt-get update && apt-get install -y --no-install-recommends libstdc++6 && rm -rf /var/lib/apt/lists/*

# Copy compiled C++ binary from builder stage
COPY --from=cpp-builder /build/cpp_engine/bin/sars_engine ./cpp_engine/bin/sars_engine
RUN chmod +x ./cpp_engine/bin/sars_engine

# Copy package configurations and install production dependencies
COPY package.json ./
COPY server/package*.json ./server/
RUN cd server && npm install --omit=dev

# Copy application assets
COPY server/ ./server/
COPY public/ ./public/

# Runtime environment settings for Render / Cloud Containers
ENV HOST=0.0.0.0
ENV PORT=3000
ENV NODE_ENV=production
ENV ENGINE_PATH=/app/cpp_engine/bin/sars_engine

EXPOSE 3000

# Start Express Application Server
CMD ["node", "server/server.js"]
