uv installation notes

This devcontainer tries to provide the real `uv` tool (Astral's `uv`) at build time using `pipx` when possible, and falls back to a lightweight development stub named `uvx` if the real tool cannot be installed.

What happens during build:
- Dockerfile attempts: `python3 -m pip install pipx && python3 -m pipx install uv`
- If `uv` is installed and `uvx` is expected by a runner, the Dockerfile creates a shim `/usr/local/bin/uvx` that points to the `uv` binary.
- If installation fails, a `uvx` stub script is placed at `/usr/local/bin/uvx` which logs invocations to `/var/log/uvx-stub.log` and emulates a long-running daemon (useful for local testing but not production).

Runtime notes (postStartCommand):
- After the container starts, the devcontainer `postStartCommand` will attempt to create a shim (if `uv` exists) or print an actionable message explaining how to install `uv` manually:
  `python3 -m pip install --user pipx && python3 -m pipx install uv`

How to replace the stub with the upstream installation:
- If you prefer the upstream install method, update the Dockerfile with the recommended steps from https://docs.astral.sh/uv/getting-started/installation/ (or the project install script) and rebuild the container.

Testing inside the container:
- `command -v uvx` - should print the path to `uvx` (either shim or stub)
- `uv --version` - should show the installed uv version (if real uv is installed)
- `tail -n 200 /var/log/uvx-stub.log` - view stub invocations (only if the stub is used)

If you would like, I can change the Dockerfile to use the upstream installation method verbatim from the Astral docs. Just confirm and I'll add it (or provide a PR with both options and a build-arg to choose).