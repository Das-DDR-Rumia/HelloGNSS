# HelloGNSS

[ English | [中文](README_zh.md) ]

A CMake-based GNSS project with an interactive CLI tool for building and managing the project.

## Prerequisites

- **CMake** (version 3.10 or higher)
- **C Compiler** (GCC, Clang, or MSVC)
- **Python 3** (for the CLI tool)
- **Git** (for cloning submodules)

## Getting Started

### 1. Clone the Repository

```bash
git clone --recursive <repository-url>
cd HelloGNSS
```

If you already cloned without `--recursive`, initialize submodules manually:

```bash
git submodule update --init --recursive
```

### 2. Install Python Dependencies

```bash
pip install typer questionary rich
```

### 3. Build the Project

#### Option A: Using the Interactive CLI Tool

```bash
python cli.py interactive
```

Then select from the menu:
1. `init-build-dir` - Initialize build directory and configure CMake
2. `build-all` - Build all targets
3. `build-target` - Build a specific target
4. `run-tests` - Run tests
5. `configure` - Configure with custom options

#### Option B: Manual Build

```bash
mkdir build
cd build
cmake ..
cmake --build .
```

## CLI Tool Commands

| Command | Description |
|---------|-------------|
| `init-build-dir` | Create build directory and initialize CMake configuration |
| `build-all` | Build all targets using CMake |
| `build-target` | Build a specific target using CMake |
| `clean-all` | Clean all build artifacts |
| `clean-target` | Clean a specific target |
| `run-tests` | Run all tests using CTest |
| `configure` | Configure CMake project with specific options |
| `exit` | Exit the CLI tool |

## Project Structure

```
HelloGNSS/
├── CMakeLists.txt      # Main CMake configuration
├── cli.py              # Interactive CLI tool
├── src/
│   └── main.c          # Main source file
├── lib/
│   └── RTKLIB/         # RTKLIB submodule
└── build/              # Build directory (generated)
```

## License

See LICENSE file for details.