# Stage 1: Compile C++17 Engine
FROM gcc:12 AS cpp-builder
WORKDIR /build
COPY cpp_engine/include ./cpp_engine/include
COPY cpp_engine/src ./cpp_engine/src
COPY cpp_engine/Makefile ./cpp_engine/Makefile
RUN cd cpp_engine && CXX=g++ make all

# Stage 2: Node.js Runtime & Application Server
FROM node:20-slim
WORKDIR /app

# Install standard C++ runtime libraries
RUN apt-get update && apt-get install -y --no-install-recommends libstdc++6 && rm -rf /var/lib/apt/lists/*

# Copy compiled C++ binary
COPY --from=cpp-builder /build/cpp_engine/bin/sars_engine ./cpp_engine/bin/sars_engine

# Copy dependencies and install
COPY server/package*.json ./server/
RUN cd server && npm install --production

# Copy application files
COPY server/ ./server/
COPY public/ ./public/

# Environment configuration
ENV PORT=3000
ENV NODE_ENV=production
EXPOSE 3000

CMD ["node", "server/server.js"]
