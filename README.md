<div align="center">

# ⚡ gitget

### Grab any file from any Git repository in milliseconds — without cloning.

[![Release](https://img.shields.io/github/v/release/sapirrior/gitget?style=flat-square&color=blue)](https://github.com/sapirrior/gitget/releases)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg?style=flat-square)](LICENSE)
[![Platform](https://img.shields.io/badge/Platform-Linux%20%7C%20Windows%20%7C%20Termux-blueviolet?style=flat-square)](https://github.com/sapirrior/gitget/releases)
[![Build Status](https://img.shields.io/badge/Build-Passing-brightgreen?style=flat-square)](https://github.com/sapirrior/gitget/actions)

<p align="center">
  <b>Tired of waiting 3 minutes to clone a 2GB repository just to grab a single config file, Makefile, or Dockerfile?</b><br>
  <code>gitget</code> fetches individual files directly into your workspace with zero overhead, no Git bloat, and instant startup time.
</p>

</div>

---

## 🎯 Why gitget?

| Traditional Git Clone | With gitget |
| :--- | :--- |
| ⏳ Downloads entire history, branches, and packfiles (hundreds of MBs) | ⚡ Fetches only the single byte stream you asked for (KBs) |
| 💽 Clutters your local filesystem with `.git/` directories | 📁 Drops clean files exactly where you want them |
| 🐌 Heavy dependencies & high network latency in CI/CD | 🚀 Instant transfer powered by high-performance C++ & `libcurl` |
| 🧩 Requires manually converting web URLs to raw URLs | 🧠 Just paste the web browser link — it figures it out automatically |

---

## ✨ Features You'll Love

- **🔗 Paste and Go (Smart URL Parsing)**  
  Copy a link straight from your browser bar (GitHub, GitLab, Bitbucket, Codeberg) and paste it into your terminal. No manual URL massaging required.
- **🌐 Universal Provider Support**  
  Works out of the box with GitHub, GitLab, Bitbucket, Codeberg/Forgejo, or any custom raw Git server.
- **🔐 Frictionless Authentication**  
  Effortlessly download from private corporate repos using your existing environment tokens (`GITHUB_TOKEN`, `GITLAB_TOKEN`, etc.) or the `-t` flag.
- **💻 Lightweight & Native Everywhere**  
  Single binary with zero runtime dependencies. Runs natively on Linux (AMD64 & ARM64/Termux) and Windows.
- **🛠️ Pipeline Ready**  
  Designed for developer scripts, Dockerfiles, dev containers, and CI/CD pipelines where speed and minimal footprint matter.

---

## 🚀 Quick Start (Under 10 Seconds)

### Download Any File Using Browser Link
```bash
gitget https://github.com/torvalds/linux/blob/master/README
```
*Done. `README` is saved directly to your current directory.*

---

## 💡 Practical Use Cases

### 1. Grab Templates, Dockerfiles, or CI Workflows
Ever find a great Dockerfile or workflow in another repo? Grab it in one command:
```bash
gitget https://github.com/facebook/react/blob/main/.editorconfig
```

### 2. Download from GitLab, Bitbucket, or Codeberg
```bash
# GitLab project
gitget https://gitlab.com/gitlab-org/gitlab-runner/-/blob/main/README.md

# Codeberg / Forgejo
gitget https://codeberg.org/forgejo/forgejo/src/branch/forgejo/README.md
```

### 3. Fetch from Private Repositories
Set your token once or pass it directly:
```bash
# Automatic token pickup via environment
export GITHUB_TOKEN="ghp_yourTokenHere"
gitget https://github.com/your-org/private-repo/blob/main/service-config.env

# Or inline flag
gitget -t ghp_yourTokenHere -r your-org/private-repo -p secrets.json
```

### 4. Custom Output Path
```bash
gitget -r torvalds/linux -p Makefile -b master -o kernel.mk
```

---

## 📦 Installation

Pre-built standalone binaries are published for every release on [GitHub Releases](https://github.com/sapirrior/gitget/releases).

### Quick Install (Linux, Termux)
```bash
curl -fsSL https://raw.githubusercontent.com/sapirrior/gitget/main/installer/install.sh -o install.sh && bash install.sh && rm -f install.sh
```

### Quick Install (Windows PowerShell)
```powershell
Invoke-WebRequest https://raw.githubusercontent.com/sapirrior/gitget/main/installer/install.ps1 -OutFile install.ps1; & .\install.ps1; Remove-Item install.ps1 -Force
```

### Build from Source (Go)
```bash
git clone https://github.com/sapirrior/gitget.git
cd gitget
go build -ldflags="-s -w" ./cmd/gitget
```
Or directly via `go install`:
```bash
go install github.com/sapirrior/gitget/cmd/gitget@latest
```

---

## 🎛️ Command-Line Options

```text
Usage: gitget [git-url] [options]
       gitget -r <owner/repo> -p <file/path> [options]

  -r, --repo <owner/repo>     Target repository (e.g. torvalds/linux)
  -p, --path <file/path>      Path of the file inside the repository
  -u, --url <url>             Direct web or raw Git file URL
  -t, --token <token>         Personal Access Token for private repositories
  -P, --provider <name>       Git provider: github, gitlab, bitbucket, codeberg, raw
  -b, --branch <branch>       Branch, tag, or commit SHA (default: main)
  -o, --output <file>         Destination filename or path
  -h, --help                  Show help information
```

---

## 📄 License

Distributed under the [MIT License](LICENSE).  
Copyright © 2026 [sapirrior](https://github.com/sapirrior).
