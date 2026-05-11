# IDA Pro 9.3 Linux 汉化补丁 (IDA Pro 9.3 Linux Translation Patch)

本项目提供了一个适用于 Linux 环境下 IDA Pro 9.3 的非破坏性汉化方案。通过动态拦截 Qt6 的底层字符串处理函数，结合外部翻译字典，在运行时实现界面汉化，无需修改任何官方二进制文件。

## 🌟 核心原理

在 Windows 系统下，汉化通常通过 DLL 劫持（如 `winmm.dll`）来实现。在 Linux 环境下，本项目通过更原生的 `LD_PRELOAD` 机制加载自定义的动态链接库 (`libidahook.so`)。该动态库会 Hook IDA 内部使用的 Qt6 框架的核心翻译函数：
- `QCoreApplication::translate`
- `QString::fromUtf8`

当 IDA 尝试显示界面文本时，Hook 库会自动在 `language.txt` 字典中查找并替换为中文。

## 📂 文件说明

- `hook.cpp`: 核心 Hook 代码实现。
- `language.txt`: 汉化字典文件（包含英文原文与中文翻译的 CSV/UTF-8 映射）。
- `Makefile`: 构建脚本，用于快速编译动态库。
- `libidahook.so`: 编译好的动态库（可直接使用）。

## 🚀 使用方法

### 1. 直接使用（推荐）

确保 `libidahook.so` 和 `language.txt` 在**同一个目录**下。

在终端中，通过设置 `LD_PRELOAD` 环境变量来启动 IDA Pro（请将路径替换为您实际的路径）：

```bash
LD_PRELOAD=/你的/路径/libidahook.so /你的/IDA/路径/ida
```

例如：
```bash
LD_PRELOAD=/home/ubuntu/idapro93汉化/libidahook.so /home/ubuntu/ida-pro-9.3/ida
```

成功加载后，终端会输出类似以下的提示：
```text
[IDA Hook] Loaded 3054 translations from /home/ubuntu/idapro93汉化/language.txt
```

### 2. 从源码编译

如果您想自行修改源码或在不同的 Linux 发行版上重新编译，只需执行以下命令：

```bash
make clean
make
```
编译完成后，会在当前目录下生成新的 `libidahook.so` 文件。

## 📄 开源协议

本项目采用 [MIT License](LICENSE) 开源协议。您可以自由地使用、修改和分发，但请保留原作者的版权声明。

## ⚠️ 免责声明

本项目仅供学习、研究和技术交流使用。IDA Pro 软件的版权归 Hex-Rays SA 所有。请支持正版软件。使用本补丁产生的一切后果由使用者自行承担。
