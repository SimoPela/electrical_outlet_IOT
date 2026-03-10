#!/usr/bin/env bash
set -u

PROJECT_DIR="$(pwd)"
IDF_PATH="${HOME}/.platformio/packages/framework-espidf"
EXPORT_SH="${IDF_PATH}/export.sh"
INSTALL_SH="${IDF_PATH}/install.sh"
IDF_PY="${IDF_PATH}/tools/idf.py"
CLANGD_FILE="${PROJECT_DIR}/.clangd"
VSCODE_DIR="${PROJECT_DIR}/.vscode"
VSCODE_SETTINGS="${VSCODE_DIR}/settings.json"

echo "==> Project: ${PROJECT_DIR}"

if [[ ! -d "${IDF_PATH}" ]]; then
    echo "ERROR: framework-espidf not found in ${IDF_PATH}"
    exit 1
fi

if [[ ! -f "${EXPORT_SH}" ]]; then
    echo "ERROR: export.sh not found in ${EXPORT_SH}"
    exit 1
fi

if [[ ! -f "${IDF_PY}" ]]; then
    echo "ERROR: idf.py not found in ${IDF_PY}"
    exit 1
fi

echo "==> Making idf.py executable"
chmod +x "${IDF_PY}"

PY_ENV_DIR="$(find "${HOME}/.espressif/python_env" -maxdepth 1 -type d -name 'idf*_py*_env' 2>/dev/null | head -n 1 || true)"

if [[ -z "${PY_ENV_DIR}" ]]; then
    echo "==> ESP-IDF Python environment missing, running install.sh"
    bash "${INSTALL_SH}" || {
        echo "ERROR: install.sh failed"
        exit 1
    }
else
    echo "==> ESP-IDF Python environment found: ${PY_ENV_DIR}"
fi

echo "==> Activating ESP-IDF"
# shellcheck disable=SC1090
source "${EXPORT_SH}" || {
    echo "ERROR: failed to source export.sh"
    exit 1
}

echo "==> Checking ESP-IDF version"
python "${IDF_PY}" --version || {
    echo "ERROR: ESP-IDF python command failed"
    exit 1
}

echo "==> Running build to generate compile_commands.json"
python "${IDF_PY}" build || true

if [[ ! -f "${PROJECT_DIR}/build/compile_commands.json" ]]; then
    echo "ERROR: build/compile_commands.json was not generated"
    exit 1
fi

echo "==> Writing .clangd"
cat > "${CLANGD_FILE}" <<'EOF'
CompileFlags:
  CompilationDatabase: build
EOF

echo "==> Writing .vscode/settings.json"
mkdir -p "${VSCODE_DIR}"
cat > "${VSCODE_SETTINGS}" <<'EOF'
{
  "C_Cpp.intelliSenseEngine": "disabled",
  "C_Cpp.errorSquiggles": "disabled",
  "clangd.arguments": [
    "--compile-commands-dir=build"
  ]
}
EOF

echo
echo "DONE"
echo "- clangd configured"
echo "- compile_commands.json found"
echo "- cpptools disabled at workspace settings level"
echo
echo "If Cursor still shows fake errors:"
echo "1. disable Microsoft C/C++ extension for this workspace"
echo "2. run: Reload Window"