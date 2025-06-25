# Spline to mesh

一个简单易用的工具，用于将[Gismo库](https://github.com/gismo/gismo)中的样条转换为 Surface Mesh.

目前支持单个二维、三维样条的转换，输出为表面网格。

## Dependency

### Basic
- [CMake](https://cmake.org/)
- 以下任意一个 C++ 编译器:
    - [MSVC](https://visualstudio.microsoft.com/zh-hans/vs/features/cplusplus/)
    - [Clang](https://clang.llvm.org/)
    - [GCC](https://gcc.gnu.org/)
### Library
- [Gismo](https://github.com/gismo/gismo)
## How to compile
```shell
cd /path/to/project
cmake -B build

cmake --build build --target Spline_to_mesh --config Release
```

之后，可以在 `bin` 目录下通过以下命令运行：
```shell
./Spline_to_mesh <INPUT_XML> -o <OUTPUT_MESH>
```
可以使用 `-h` 参数输出程序参数说明：
```shell
./Spline_to_mesh -h
```
可选参数说明：
- `-o`：后接输出网格文件路径，默认为 `output.off`.
- `-n`：后接每个方向网格数，默认为 64.
- `--color`：标志位，表示输出网格时是否包含颜色信息，默认为 false.
- `--invert`：标志位，表示是否反转网格法线，默认为 false.

## What's next

- [ ] 使用 `gsMesh`，支持导出四边形网格
- [ ] 支持更多的网格类型
- [ ] 支持一维样条转换为线网格
- [ ] 支持多块样条转换
- [ ] 支持配置文件导入部分参数
