# CeyhanAudioCore (CAC) 🚀
[![GitHub Issues](https://img.shields.io/github/issues/istek26/CeyhanAudioCore?style=for-the-badge)](https://github.com/istek26/CeyhanAudioCore/issues)
[![Contribute](https://img.shields.io/badge/Contributions-Welcome-brightgreen?style=for-the-badge)](https://github.com/istek26/CeyhanAudioCore/pulls)

CeyhanAudioCore is a lightweight, high-performance, open-source audio copyright and fingerprinting kernel written in C++17. It continuously scans audio files, processes raw sound waves, and generates a unique mathematical digital fingerprint (`CAC-ID`) to protect intellectual property and detect copyright infringement.



## Key Features 🌟
* **Continuous Scanning Daemon:** Processes multiple files sequentially without restarting.
* **Deterministic Fingerprinting:** Generates unique hash codes based entirely on audio data bytes, completely ignoring file name alterations.
* **Hacker-Style Interactive Terminal UI:** Lightweight, responsive, and color-coded environment designed for server/backend integration.
* **Zero External Dependencies:** Built purely using standard C++ libraries (`STL`), making it ultra-portable across macOS and Linux.

## Technical Specifications 🛠️
* **Language:** C++17
* **Core Architecture:** Wave-byte parsing & custom hashing algorithm (`DJB2` variance)
* **Target OS:** macOS (Optimized for Apple Silicon / Intel MacBook Pro), Linux

## Quick Start & Compilation 💻

Ensure you have a C++17 compliant compiler installed (like Clang or GCC).

1. Clone the repository and navigate into the directory:
   ```bash
   cd CeyhanAudioCore
