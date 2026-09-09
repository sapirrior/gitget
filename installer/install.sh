#!/usr/bin/env bash
set -e

REPO="sapirrior/gitget"
BINARY_NAME="gitget"

# Styling helpers
BOLD="\033[1m"
GREEN="\033[0;32m"
BLUE="\033[0;34m"
YELLOW="\033[0;33m"
RED="\033[0;31m"
RESET="\033[0m"

# 1. Detect OS & Environment
IS_TERMUX=false
if [ -n "${TERMUX_VERSION:-}" ] || [ -d "/data/data/com.termux" ]; then
    IS_TERMUX=true
fi

OS="$(uname -s)"
case "${OS}" in
    Linux*)
        OS_TYPE="linux"
        ;;
    Darwin*)
        OS_TYPE="macos"
        ;;
    MSYS*|MINGW*|CYGWIN*)
        OS_TYPE="windows"
        BINARY_NAME="gitget.exe"
        ;;
    *)
        echo -e "${RED}Error: Unsupported operating system '${OS}'.${RESET}"
        exit 1
        ;;
esac

# 2. Detect Architecture
ARCH="$(uname -m)"
case "${ARCH}" in
    x86_64|amd64)
        ARCH_TYPE="amd64"
        ;;
    aarch64|arm64)
        ARCH_TYPE="arm64"
        ;;
    *)
        echo -e "${RED}Error: Unsupported architecture '${ARCH}'.${RESET}"
        exit 1
        ;;
esac

ASSET_NAME="gitget-${OS_TYPE}-${ARCH_TYPE}"
if [ "${OS_TYPE}" = "windows" ]; then
    ASSET_NAME="${ASSET_NAME}.exe"
fi

echo -e "Detected System: ${GREEN}${OS_TYPE} (${ARCH_TYPE})${RESET}"
echo -e "Target Artifact: ${GREEN}${ASSET_NAME}${RESET}\n"

# 3. Confirmation to proceed with download
read -r -p "Do you want to proceed with downloading '${ASSET_NAME}'? [Y/n] " CONFIRM_DOWNLOAD
CONFIRM_DOWNLOAD="${CONFIRM_DOWNLOAD:-y}"
if [[ ! "${CONFIRM_DOWNLOAD}" =~ ^[Yy]$ ]]; then
    echo "Installation aborted by user."
    exit 0
fi

# 4. Fetch latest release download URL
DOWNLOAD_URL="https://github.com/${REPO}/releases/latest/download/${ASSET_NAME}"

# Temp location
TMP_DIR="$(mktemp -d)"
TMP_FILE="${TMP_DIR}/${BINARY_NAME}"
cleanup() {
    rm -rf "${TMP_DIR}"
}
trap cleanup EXIT

echo -e "\nDownloading ${BLUE}${DOWNLOAD_URL}${RESET}..."
if command -v curl >/dev/null 2>&1; then
    curl -fsSL -o "${TMP_FILE}" "${DOWNLOAD_URL}"
elif command -v wget >/dev/null 2>&1; then
    wget -qO "${TMP_FILE}" "${DOWNLOAD_URL}"
else
    echo -e "${RED}Error: Neither curl nor wget was found in PATH.${RESET}"
    exit 1
fi
chmod +x "${TMP_FILE}"

echo -e "${GREEN}✓ Download complete!${RESET}\n"

# 5. Prompt installation destination
echo -e "${BOLD}Where would you like to install gitget?${RESET}"
if [ "${IS_TERMUX}" = true ]; then
    echo "  1) Termux bin in ${PREFIX:-/data/data/com.termux/files/usr}/bin (recommended)"
    echo "  2) User local in \$HOME/.local/bin"
    echo "  3) Current directory ($(pwd)) as '${BINARY_NAME}'"
    echo "  4) Custom directory"
    read -r -p "Select an option [1-4] (default: 1): " DEST_CHOICE
    DEST_CHOICE="${DEST_CHOICE:-1}"
else
    echo "  1) System-wide in /usr/local/bin (requires sudo)"
    echo "  2) User local in \$HOME/.local/bin"
    echo "  3) Current directory ($(pwd)) as '${BINARY_NAME}'"
    echo "  4) Custom directory"
    read -r -p "Select an option [1-4] (default: 2): " DEST_CHOICE
    DEST_CHOICE="${DEST_CHOICE:-2}"
fi

case "${DEST_CHOICE}" in
    1)
        if [ "${IS_TERMUX}" = true ]; then
            TARGET_DIR="${PREFIX:-/data/data/com.termux/files/usr}/bin"
        else
            TARGET_DIR="/usr/local/bin"
        fi
        ;;
    2)
        TARGET_DIR="${HOME}/.local/bin"
        ;;
    3)
        TARGET_DIR="$(pwd)"
        ;;
    4)
        read -r -p "Enter custom directory path: " CUSTOM_DIR
        TARGET_DIR="${CUSTOM_DIR}"
        ;;
    *)
        if [ "${IS_TERMUX}" = true ]; then
            TARGET_DIR="${PREFIX:-/data/data/com.termux/files/usr}/bin"
        else
            TARGET_DIR="${HOME}/.local/bin"
        fi
        ;;
esac

TARGET_PATH="${TARGET_DIR}/${BINARY_NAME}"

# 6. Ask permission to copy/install to target path
read -r -p "Install '${BINARY_NAME}' to '${TARGET_PATH}'? [Y/n] " CONFIRM_INSTALL
CONFIRM_INSTALL="${CONFIRM_INSTALL:-y}"
if [[ ! "${CONFIRM_INSTALL}" =~ ^[Yy]$ ]]; then
    echo "Skipping file placement. Downloaded binary was discarded."
    exit 0
fi

if [ "${IS_TERMUX}" = true ]; then
    mkdir -p "${TARGET_DIR}"
    cp "${TMP_FILE}" "${TARGET_PATH}"
    chmod +x "${TARGET_PATH}"
else
    mkdir -p "${TARGET_DIR}" 2>/dev/null || sudo mkdir -p "${TARGET_DIR}"

    if [ -w "${TARGET_DIR}" ]; then
        cp "${TMP_FILE}" "${TARGET_PATH}"
        chmod +x "${TARGET_PATH}"
    else
        echo -e "${YELLOW}Permission required to write to ${TARGET_DIR}. Requesting sudo...${RESET}"
        sudo cp "${TMP_FILE}" "${TARGET_PATH}"
        sudo chmod +x "${TARGET_PATH}"
    fi
fi

echo -e "\n${GREEN}✓ Binary successfully installed to: ${TARGET_PATH}${RESET}"

# 7. Check if target directory is in PATH; if not, ask permission to add it
if [[ ":$PATH:" != *":${TARGET_DIR}:"* ]]; then
    echo -e "\n${YELLOW}Notice: '${TARGET_DIR}' is not currently in your \$PATH.${RESET}"
    read -r -p "Would you like to add '${TARGET_DIR}' to your shell configuration? [Y/n] " CONFIRM_PATH
    CONFIRM_PATH="${CONFIRM_PATH:-y}"
    if [[ "${CONFIRM_PATH}" =~ ^[Yy]$ ]]; then
        SHELL_NAME="$(basename "${SHELL:-bash}")"
        RC_FILE=""
        case "${SHELL_NAME}" in
            bash)
                if [ -f "${HOME}/.bashrc" ]; then
                    RC_FILE="${HOME}/.bashrc"
                else
                    RC_FILE="${HOME}/.bash_profile"
                fi
                ;;
            zsh)
                RC_FILE="${HOME}/.zshrc"
                ;;
            fish)
                RC_FILE="${HOME}/.config/fish/config.fish"
                ;;
            *)
                RC_FILE="${HOME}/.profile"
                ;;
        esac

        read -r -p "Append 'export PATH=\"${TARGET_DIR}:\$PATH\"' to '${RC_FILE}'? [Y/n] " CONFIRM_RC
        CONFIRM_RC="${CONFIRM_RC:-y}"
        if [[ "${CONFIRM_RC}" =~ ^[Yy]$ ]]; then
            if [ "${SHELL_NAME}" = "fish" ]; then
                mkdir -p "$(dirname "${RC_FILE}")"
                echo "fish_add_path ${TARGET_DIR}" >> "${RC_FILE}"
            else
                echo -e "\n# gitget binary path\nexport PATH=\"${TARGET_DIR}:\$PATH\"" >> "${RC_FILE}"
            fi
            echo -e "${GREEN}✓ Updated ${RC_FILE}.${RESET} Run ${BOLD}source ${RC_FILE}${RESET} or restart your shell."
        else
            echo "Skipped updating shell configuration file."
        fi
    fi
fi

echo -e "\n${BOLD}${GREEN}All set! Run 'gitget --help' to get started.${RESET}\n"
