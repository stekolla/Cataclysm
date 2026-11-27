FROM emscripten/emsdk:6.0.8

ARG PLAYWRIGHT_VERSION=1.64.1

ENV DEBIAN_FRONTEND=noninteractive \
    EMSDK=/emsdk \
    HOME=/tmp \
    PLAYWRIGHT_BROWSERS_PATH=/opt/playwright-browsers \
    CHROME_BIN=/usr/local/bin/cata-chromium

RUN apt-get update \
    && apt-get install -y --no-install-recommends \
        build-essential \
        ca-certificates \
        cmake \
        curl \
        git \
        gnupg \
        libncurses-dev \
        libsqlite3-dev \
        ninja-build \
        python3 \
        python3-websocket \
        g++-14 \
        zlib1g-dev \
    && rm -rf /var/lib/apt/lists/*

RUN curl -fsSL https://apt.llvm.org/llvm-snapshot.gpg.key \
        | gpg --dearmor -o /usr/share/keyrings/llvm-archive-keyring.gpg \
    && echo "deb [signed-by=/usr/share/keyrings/llvm-archive-keyring.gpg] https://apt.llvm.org/noble/ llvm-toolchain-noble-22 main" \
        > /etc/apt/sources.list.d/llvm.list \
    && apt-get update \
    && apt-get install -y --no-install-recommends clang-22 \
    && rm -rf /var/lib/apt/lists/*

RUN npm install --global "playwright@${PLAYWRIGHT_VERSION}" \
    && playwright install --with-deps chromium \
    && chrome_path="$(find "${PLAYWRIGHT_BROWSERS_PATH}" -type f -name chrome -print -quit)" \
    && test -n "${chrome_path}" \
    && ln -s "${chrome_path}" "${CHROME_BIN}" \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /workspace

RUN git config --system --add safe.directory /workspace
