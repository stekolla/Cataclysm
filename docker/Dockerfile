FROM emscripten/emsdk:6.0.8

ARG DENO_VERSION=2.9.8
ARG PLAYWRIGHT_VERSION=1.64.1
ARG TARGETARCH

ENV DEBIAN_FRONTEND=noninteractive \
    EMSDK=/emsdk \
    HOME=/tmp \
    DENO_DIR=/tmp/cata-cache/deno \
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
        unzip \
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

RUN case "${TARGETARCH}" in \
        amd64) deno_arch=x86_64 ;; \
        arm64) deno_arch=aarch64 ;; \
        *) echo "Unsupported Deno architecture: ${TARGETARCH}" >&2; exit 1 ;; \
    esac \
    && curl -fsSL "https://github.com/denoland/deno/releases/download/v${DENO_VERSION}/deno-${deno_arch}-unknown-linux-gnu.zip" -o /tmp/deno.zip \
    && unzip -q /tmp/deno.zip deno -d /usr/local/bin \
    && rm /tmp/deno.zip \
    && npm install --global "playwright@${PLAYWRIGHT_VERSION}" \
    && playwright install --with-deps chromium \
    && chrome_path="$(find "${PLAYWRIGHT_BROWSERS_PATH}" -type f -name chrome -print -quit)" \
    && test -n "${chrome_path}" \
    && ln -s "${chrome_path}" "${CHROME_BIN}" \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /workspace

RUN git config --system --add safe.directory /workspace
