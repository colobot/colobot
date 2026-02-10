# See https://containers.dev/ for details on devcontainer images
FROM mcr.microsoft.com/devcontainers/base:ubuntu-24.04

# Install build tools, runtime deps and libuv
ARG INSTALL_UV=1
RUN apt-get update && \
    apt-get install -y --no-install-recommends \
      build-essential cmake git wget curl ca-certificates pkg-config \
        libsdl2-dev libsdl2-image-dev libsdl2-ttf-dev libsndfile1-dev libvorbis-dev libogg-dev libpng-dev libglew-dev libopenal-dev libphysfs-dev gettext po4a vorbis-tools libglm-dev libuv1-dev \
      gdb lldb ninja-build python3 python3-pip pipx python3-venv && \
    rm -rf /var/lib/apt/lists/*

# Optional: install vcpkg for convenience (can be removed if you prefer system deps)
ENV VCPKG_ROOT=/usr/local/vcpkg
RUN git clone https://github.com/microsoft/vcpkg.git $VCPKG_ROOT && \
    $VCPKG_ROOT/bootstrap-vcpkg.sh

# Install the upstream uv (Astral) using their installer when requested.
# Controlled by the build arg INSTALL_UV (default 1). Try upstream installer
# to /usr/local/bin (no PATH modification), then pipx fallback, then a stub.
RUN if ! command -v uv >/dev/null 2>&1; then \
    if [ "${INSTALL_UV}" = "1" ]; then \
      echo "Attempting upstream Astral 'uv' installer..."; \
      XDG_BIN_HOME=/usr/local/bin UV_NO_MODIFY_PATH=1 \
        curl -fsSL https://astral.sh/uv/install.sh -o /tmp/uv-install.sh && \
        sh /tmp/uv-install.sh --quiet && rm -f /tmp/uv-install.sh || true; \
    else \
      echo "INSTALL_UV disabled; skipping upstream installer."; \
    fi; \
  fi

RUN if ! command -v uv >/dev/null 2>&1; then \
    echo "Attempting pipx fallback for uv (apt-installed pipx)..."; \
    pipx --version >/dev/null 2>&1 || true; \
    pipx ensurepath || true; \
    (pipx install uv || pipx install uv-cli || true); \
  fi

RUN if ! command -v uv >/dev/null 2>&1; then \
    echo "Warning: 'uv' not found after installer attempts. To install manually, run: python3 -m pip install --user pipx && python3 -m pipx install uv"; \
  fi

RUN if command -v uv >/dev/null 2>&1 && ! command -v uvx >/dev/null 2>&1; then \
    ln -s "$(command -v uv)" /usr/local/bin/uvx || true; \
  fi

# Keep bash as default shell
SHELL ["/bin/bash", "-lc"]

# Small healthcheck (non-blocking)
RUN cmake --version || true
