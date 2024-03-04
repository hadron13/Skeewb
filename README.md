# Skeewb
> ## A minimalist, modular Game/engine with focus on flexibility and performance 
# Compiling
## Linux (gcc/clang)
### Core
```
cc src/skeewb.c -o build/skeewb
```
### Modules
```
cc [module source files] -shared -fPIC -o [output.so]
#renderer
cc src/renderer/renderer.c  -shared -fPIC -lSDL2 -o build/mods/renderer.so
```

## Windows (MinGW)
### Core
```
cc src\skeewb.c -o build\skeewb.exe
```
### Modules
```
gcc [module source files] -shared -fPIC --export-all [output.dll]
#renderer
gcc src\renderer\renderer.c -shared -fPIC --export-all -lmingw32 -lSDL2main -lSDL2 -o build\mods\renderer.dll
```
