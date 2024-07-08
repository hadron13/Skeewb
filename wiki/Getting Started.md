This is a quick guide on modding [[Skeewb]]

## Installing
#### Prerequisites
- C compiler: gcc/clang/mingw-w64
- Cmake
- SDL3 required libraries (Linux)

#### Downloading and compiling Skeewb (Windows/Linux)

```bash
git clone https://github.com/hadron13/Skeewb.git --recurse-submodules
cd Skeewb
gcc crane.c -o crane # clang works too!
./crane
```

## Creating a module
After [[#Installing]]
```bash
./crane example
cd examplemod
gcc crane.c -o crane
./crane
```

Doing these commands will create and compile a simple example [[Module]] containing the following code:

```c
#define MODULE "Example"
#include<skeewb/skeewb.h>

module_desc_t load(core_interface_t *core){ 

	core->console_log(INFO, "hello Skeewb!");
	
	return (module_desc_t){
		.modid = str("example"),
		.version = {0, 0, 1},
	};
}
```
Breaking this code down:

```c
#define MODULE "Example"
```
Changes the displayed module using `console_log()`

```c
#include<skeewb/skeewb.h>
```
Includes the base module library

```c
module_desc_t load(core_interface_t *core)
```
Is the standard entry point for the module

```c
core->console_log(INFO, "hello Skeewb!");
```
Logs a message into the console, more info in [[Core]]

```c
	return (module_desc_t){
		.modid = str("example"),
		.version = {0, 0, 1},
	};
```
Returns the module description to the [[Core]].

## Beyond Hello Skeewb

#### Using events
```c
void start(core_interface_t *core){
	// all modules loaded by now
}

void loop(core_interface_t *core){
	// executed in a loop
}

void quit(core_interface_t *core){
	// quit() called
}

module_desc_t load(core_interface_t *core){ 
	core->event_listen(str("start"), start);
	core->event_listen(str("loop"), start);
	core->event_listen(str("quit"), start);
	
	return (module_desc_t){
		.modid = str("example"),
		.version = {0, 0, 1},
	};
}

```
This example code uses all the standard events in the [[Core]]

#### Using configs
Configs can be used for user customization and configuring other modules

```c
//set configuration
core->config_set((config_t){
	.name = str("maximum burgers"),
	.type = TYPE_INTEGER,
	.value = 10000
});

//get configuration
config_t burger = core->config_get(str("maximum burgers"));
printf("%i burgers", burger.value.integer);
```

#### Using resources
Resources are files that can be loaded and overridden by modules

```c
resource_t *funny_text = core->resource_load(str("text"), str("test/fun.txt")));
funny_text = core->resource_overload(str("text"), str("test/Joe.txt"));

string_t joe_string = core->resource_string(funny_text); // contents of Joe.txt
```
Refer to the [[Core]] documentation for more information

#### Using other module interfaces

```c
#include<skeewb/renderer.h>

void start(core_interface_t *core){
	renderer_interface_t *renderer = core->module_get_interface(str("renderer"));

	renderer->window_resize(300, 300);
}

```

All the base game modules should be available under `skeewb/` in the examplemod directory

#### Exporting your own interface

I recommend you create 2 headers: example.h and example_internal.h, with the first containing the types and the interface, and the latter internal module functions and data.

example.h
```c
typedef enum {CHEESE_BURGER, BACON_BURGER, FISH_BURGER} burger_type_t;

typedef struct{
	burger_type_t type;
	int size;
}burger_t;

typedef struct{
	burger_t (*burger_generate)(int budget);
}example_interface_t;

```

example_internal.h
```c
#include "example.h"

example_interface_t example_interface = {
	.burger_generate = example_burger_generate
};
```

example.c
```c
#include "example_internal.h"

module_desc_t load(core_interface_t *core){ 
	return (module_desc_t){
		.modid = str("example"),
		.version = {0, 0, 1},
		.interface = &example_interface
	};
}
```

#### Code Convention
I encourage you to follow the [[Code Convention]] for better consistency among all modules.

#### Exploring other modules
The base game module's _will_ contain useful code in modding, ranging from a simple popup to whole rendering pipelines.
These are (_or will be_) the base modules:
- [[Renderer]]
- [[UI]]
- [[World]]
- [[Worldgen]]
- [[Entity]]
- [[Skeewb-Game]]