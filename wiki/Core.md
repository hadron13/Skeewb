The Core is the entry point of [[Skeewb]], and is responsible for loading all modules

It is implemented in a single file at `src/skeewb.c`

### Types
Types declared at `src/skeewb.h` or `skeewb/skeewb.h` in the example mod
- `shared_object_t` - shared library handle
- `function_pointer_t` - function pointer handle
- `interface_t` - [[Module]] interface handle
- `module_t` - module handle
- `event_t` - event handle
- `event_callback_t` - event callback function pointer equivalent to `void f(void *data)`
- `log_category_t` - logging category enum: `INFO`, `WARNING`, `ERROR`, etc.
- `config_type_t` - configuration type enum: `EMPTY`, `TYPE_INTEGER`, `TYPE_REAL`, `TYPE_STRING`, `TYPE_BOOLEAN`. 
- `version_t` - version struct following major.minor.patch format
- `module_desc_t` - module descriptor
- `config_value_t` - configuration value union
- `config_t` - configuration entry
- `resource_t` - resource entry
- `core_interface_t` - core interface type

## Interface

The core includes a set of functionalities, all included in the `core_interface_t` struct, they can be called using `core->function()` using the pointer provided

A [[Module]] can override these function pointers if needed.

### Logging
- `void console_log(log_category_t category, char *restrict format, ...)` - printf-like logging function, is overriden with a macro to include line, file and caller function.
##### Example
```c
core->console_log(WARNING, "%is to explode", 5);
```
Output: `[WARNING][ Example | src/example/nuke.c:123 countdown()] 5s to explode`

Note: "Example" will only appear if the macro MODULE is set.

### Events
Events are simple callback lists managed by the core.
- `void event_register(const string_t name)` - registers a new event
- `void event_listen(const string_t name, event_callback_t callback)` - listen to function with callback
- `void event_trigger(const string_t name, void *context)` - trigger event, calling all functions with the context as the argument.
##### Example
```c
void who(void *data){
	printf("who's there? %s", (char*) data);
}
...
core->event_register(str("knock"));
core->event_listen(str("knock"), who);
core->event_trigger(str("knock"), "Joe");
```
Output: `who's there? Joe`

Note: all strings are copied, you can deallocate just after calling any of them.

#### Core Events
Three events are registered by the core itself:
- `start` - triggered after all modules have been loaded
- `loop` - triggered in a loop until quit() is called
- `quit` - triggered with quit()

### Configs
- `void config_set(config_t config)` - set a configuration
- `config_t config_get(const string_t name)` - retrieve a configuration
##### Example
```c
core->config_set((config_t){
	.name = str("maximum burgers"),
	.type = TYPE_INTEGER,
	.value = 10000
});
config_t burger = core->config_get(str("maximum burgers"));
printf("%i burgers", burger.value.integer);
```
### Resources
- `resource_t *resource_load(const string_t name, const string_t path)` - loads a resource, if already loaded somewhere else will return that version instead
- `resource_t *resource_overload(const string_t name, const string_t new_path)` - same as `resource_load()`, however will override the resource regardless
- `string_t resource_string(resource_t *resource)` - loads resource into string buffer.

##### Example
```c
resource_t *funny_text = core->resource_load(str("text"), str("test/fun.txt")));
funny_text = core->resource_load(str("text"), str("test/cien.txt")));

string_t funny_string = core->resource_string(funny_text); // contents of fun.txt

funny_text = core->resource_overload(str("text"), str("test/Joe.txt"));

string_t joe_string = core->resource_string(funny_text); // contents of Joe.txt
```
Note: resources are loaded from module's directory, `test/cien.txt` may be in `mods/example/test/cien.txt`

### Module Manipulation
- `version_t module_get_version(string_t modid)` - get a module's version by modid
- `interface_t *module_get_interface(string_t modid)` - get a module's interface with modid
- `function_pointer_t *module_get_function(string_t modid, string_t name)` - get a module's function using the underlying OS functions
- `void module_reload(string_t modid)` - reload a module native object and call `reload()` function
##### Example
```c
version_t version = core->module_get_version(str("Gregtech"));
printf("Greg %i.%i.%i", version.major, version.minor, version.patch);

//greg_interface_t must be provided by Gregtech
greg_interface_t *interface = core->module_get_interface(str("Gregtech"));
interface->greg();

void (*spooky)(machine_t *machine) = core->module_get_function(str("Gregtech"), str("spooky"));
spooky(NULL);

core->module_reload(str("Gregtech"));
```
Note: Windows DLLs _may_ not export all functions

### Miscellaneous
- `void quit(int status)` - quits application with status, triggers `quit` event