# Skeewb
> ## A minimalist, modular voxel game/engine with focus on flexibility and performance 
Surpassed by [Slate](https://github.com/hadron13/sSlate), basically same thing but written in odin, by an older & wiser me
# Modules

The whole engine is split into different, swappable modules, which are libraries (.dll/.so) loaded at runtime by the core executable.

Each Module can communicate with other modules by the core config/resource/event system and interfaces.
# Compiling

## Prerequisites
- C compiler: gcc/clang/[mingw-w64](https://github.com/niXman/mingw-builds-binaries/releases)
- [Odin Compiler](https://github.com/odin-lang/Odin/releases/) (soon)
- [Cmake](https://cmake.org/download/)
- SDL required libraries (Linux)

## Linux (gcc/clang) / Windows (MinGW)
```bash
git clone https://github.com/hadron13/Skeewb.git --recurse-submodules
cd Skeewb
gcc crane.c -o crane # clang works too 
./crane              # may be .exe in Windows
```

# Creating a module

An example module can be created using the following commands:
```bash
./crane example
cd examplemod
gcc crane.c -o crane # examplemod's own crane
./crane
```

Check the [wiki](https://github.com/hadron13/Skeewb/wiki) for more information.

