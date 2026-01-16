# README_zh.md

# HelloGNSS

[ [English](README.md) | 中文 ]

基于 CMake 的 GNSS 项目，附带交互式 CLI 工具用于构建和管理项目。

## 环境要求

- **CMake**（3.10 或更高版本）
- **C 编译器**（GCC、Clang 或 MSVC）
- **Python 3**（用于 CLI 工具）
- **Git**（用于克隆子模块）

## 快速开始

### 1. 克隆仓库

```bash
git clone --recursive <仓库地址>
cd HelloGNSS
```

如果克隆时未使用 `--recursive`，请手动初始化子模块：

```bash
git submodule update --init --recursive
```

### 2. 安装 Python 依赖

```bash
pip install typer questionary rich
```

### 3. 构建项目

#### 方式 A：使用交互式 CLI 工具

```bash
python cli.py interactive
```

然后从菜单中选择：
1. `init-build-dir` - 初始化构建目录并配置 CMake
2. `build-all` - 构建所有目标
3. `build-target` - 构建特定目标
4. `run-tests` - 运行测试
5. `configure` - 使用自定义选项配置

#### 方式 B：手动构建

```bash
mkdir build
cd build
cmake ..
cmake --build .
```

## CLI 工具命令

| 命令 | 描述 |
|------|------|
| `init-build-dir` | 创建构建目录并初始化 CMake 配置 |
| `build-all` | 使用 CMake 构建所有目标 |
| `build-target` | 使用 CMake 构建特定目标 |
| `clean-all` | 清理所有构建产物 |
| `clean-target` | 清理特定目标 |
| `run-tests` | 使用 CTest 运行所有测试 |
| `configure` | 使用特定选项配置 CMake 项目 |
| `exit` | 退出 CLI 工具 |

## 项目结构

```
HelloGNSS/
├── CMakeLists.txt      # 主 CMake 配置文件
├── cli.py              # 交互式 CLI 工具
├── src/
│   └── main.c          # 主源文件
├── lib/
│   └── RTKLIB/         # RTKLIB 子模块
└── build/              # 构建目录（自动生成）
```

## 许可证

详见 LICENSE 文件。