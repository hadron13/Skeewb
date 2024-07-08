Modules, or mods for short, are single dynamically linked library (.so/dll) loaded at runtime by the [[Core]] executable.

## Entry point

The standard entry point is `module_desc_t load(core_interface_t *core);`
This function will be called as soon as the shared library for the module is loaded, in no specific order

## Hot Reload (WIP)

Modules can be hot reloaded through `module_reload()` in the [[Core]] interface
and this will call these 2 functions in the chosen module:
- `void prereload(core_interface_t *core)` - before reloading
- `module_desc_t reload(core_interface_t *core)` - after reloading

As soon as `module_reload()` is called, the core will call `prereload()` to prepare for reloading, the core is not responsible for any cleanup. Ater `prereload()` returns, the shared library will be unloaded and then loaded back, calling `reload()` instead of `load()`