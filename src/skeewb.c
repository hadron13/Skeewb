#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>


// #include <threads.h>

// pentagon, hexagon, MSVC's gon
//#include<pthread.h>

#define MODULE "Core"

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#define WINDOWS
#elif defined(__unix__) || defined(__unix)
#define UNIX
#endif

#ifdef UNIX
#define PATH_SEPARATOR '/'
#define PATH_SEPARATOR_STR "/"
#define DYLIB_EXTENSION ".so"
#include <dirent.h>
#include <dlfcn.h>
#include <unistd.h>
#endif

#ifdef WINDOWS
#define PATH_SEPARATOR '\\'
#define PATH_SEPARATOR_STR "\\"
#define DYLIB_EXTENSION ".dll"
#include <windows.h>
#endif

//mingw complains about these
#undef interface
#undef ERROR 

#define ICE_CPU_IMPL
#include "libs/ice_cpu.h"
#include "libs/ds.h"

/*
 *  ======== \\    // ||===\\  ||==== //===\\ 
 *     ||     \\  //  ||    || ||     ||      
 *     ||      \\//   ||===//  ||===  \\===\\ 
 *     ||       ||    ||       ||           ||
 *     ||       ||    ||       ||==== \\===// 
 */

typedef void shared_object_t;
typedef void function_pointer_t;
typedef void interface_t;

typedef void(*event_callback_t)(void *context);

typedef enum { DEBUG, VERBOSE, INFO, WARNING, ERROR, CRITICAL } log_category_t;

typedef enum { EMPTY, BOOLEAN, INTEGER, REAL, STRING } config_type_t;

typedef struct{
    char *name;
    config_type_t type;
    union{
        bool boolean;
        int64_t integer;
        double real;
        char *string;
    }value;
}config_t;


typedef struct{
    uint16_t major;
    uint16_t minor;
    uint16_t patch;
}version_t;

typedef struct{
    char             *name;
    event_callback_t *callbacks;
}event_t;

typedef struct{
    char            *name;
    version_t        version;
    char            *path;
    shared_object_t *shared_object;
    interface_t     *interface;
}module_t;

typedef struct{
    char *modid;
    version_t version;
    interface_t *interface;
}module_desc_t;

typedef struct{
    char *dependent;
    char *modid;
    version_t min;
    version_t max;
}module_requirement_t;

typedef struct{
    version_t version;
    void         (*log_)(log_category_t category, char *restrict format, ...);
    void         (*event_register)(const char *name);
    void         (*event_trigger)(const char *name, void *context);
    void         (*event_listen)(const char *name, event_callback_t callback);
    void         (*quit)(int status);
    void         (*config_set)(config_t config);
    config_t     (*config_get)(const char *name);
    void         (*require_version)(const char *modid, version_t min_version, version_t max_version);
    void         (*require)(const char *modid);
    interface_t* (*module_get_interface)(const char *modid);
}core_interface_t;


typedef module_desc_t(*start_func_t)(core_interface_t *interface);



/*
 * ====== ||    || ||\  ||  //===\\ ======== ======  //===\\  ||\  ||  //===\\ 
 * ||     ||    || ||\\ || ||          ||      ||   ||     || ||\\ || ||      
 * ||===  ||    || || \\|| ||          ||      ||   ||     || || \\||  \\===\\ 
 * ||     ||    || ||  \\| ||          ||      ||   ||     || ||  \\|        ||
 * ||      \\==//  ||   ||  \\===//    ||    ======  \\===//  ||   \|  \\===//
 */

void core_event_register(const char *name);
void core_event_trigger (const char *name, void *context);
void core_event_listen  (const char *name, event_callback_t callback);

void     core_config_set(config_t config);
config_t core_config_get(const char *name);

void         core_require_version(const char *modid, version_t min_version, version_t max_version);
void         core_require(const char *modid);

interface_t* core_module_get_interface(const char *modid);
void         core_module_reload(const char *modid);

void core_quit(int status);

void core_log_(log_category_t category, char *restrict format, ...);
#define core_log(category, format, ...) core_log_(category, "[\033[34m" MODULE" |"__FILE__ ":\033[35m%d \033[93m%s()\033[0m] "format, __LINE__, __func__ __VA_OPT__(,) __VA_ARGS__)

char                **platform_enumerate_directory(char *directory_path, bool directories); // returns dynamic array of strings
shared_object_t      *platform_library_load(char *path);
function_pointer_t   *platform_library_load_symbol(shared_object_t *object, char *name);
void                  platform_library_unload(shared_object_t *object);


void cleanup(void);

/*
 *  //===\\  ||      //===\\  ||==\\     /\     ||      //===\\ 
 * ||        ||     ||     || ||   ||   //\\    ||     ||      
 * ||  ===\\ ||     ||     || ||==\\   //  \\   ||      \\===\\ 
 * ||     || ||     ||     || ||   || //====\\  ||            ||
 *  \\===//  ======  \\===//  ||==// //      \\ ======  \\===//
 */

core_interface_t core_interface = {
    {0, 0, 1},
    .log_ = core_log_,
    .event_register = core_event_register,
    .event_trigger = core_event_trigger,
    .event_listen = core_event_listen,
    .quit = core_quit,
    .config_set = core_config_set,
    .config_get = core_config_get,
    .require_version = core_require_version,
    .require = core_require,
    .module_get_interface = core_module_get_interface,
};


str_hash_t module_hashtable;
module_t *modules;

str_hash_t event_hashtable;
event_t *events;

str_hash_t config_hashtable;
config_t *configs;



/*
 * ====== ||\  /|| ||==\\  ||     ====== ||\  /|| ====== ||\  || ========  /\  ======== ======  //===\\  ||\  ||
 *   ||   ||\\//|| ||   || ||     ||     ||\\//|| ||     ||\\ ||    ||    //\\    ||      ||   ||     || ||\\ ||
 *   ||   || \/ || ||==//  ||     ||===  || \/ || ||===  || \\||    ||   //  \\   ||      ||   ||     || || \\||
 *   ||   ||    || ||      ||     ||     ||    || ||     ||  \\|    ||  //====\\  ||      ||   ||     || ||  \\|
 * ====== ||    || ||      ====== ====== ||    || ====== ||   ||    || //      \\ ||    ======  \\===//  ||   ||
 */

int main(int argc, char **argv) {

    atexit(cleanup);

    modules = list_init(module_t);
    events =  list_init(event_t);
    configs = list_init(config_t);
    module_hashtable = str_hash_create(5);
    event_hashtable  = str_hash_create(8);
    config_hashtable = str_hash_create(8);

    core_log(INFO, "starting");

    if(argc > 1){
        core_log(INFO, "parsing arguments");
        for(size_t i = 1; i < argc; i++){
            char *current_arg = argv[i];

            if(current_arg[0] != '-' || current_arg[1] == '\0'){
                core_log(ERROR, "invalid argument: %s", current_arg);
                continue;
            }

            char *first_equal = strchr(current_arg, '=');
            if(!first_equal){ 
                core_config_set((config_t){
                    .name = current_arg + 1,
                    .type = BOOLEAN,
                    .value.boolean = true
                });
                continue;
            }   
            size_t name_length = first_equal - 1 - current_arg ; 
            size_t value_length = strlen(first_equal) - 1;
            
            if(name_length == 0){
                core_log(ERROR, "in argument \"%s\", no variable name found", current_arg);
                continue;
            }
            if(value_length == 0){
                core_log(ERROR, "in argument \"%s\", no value found", current_arg);
                continue;
            }
            
            char *name = string_duplicate_len(current_arg + 1, name_length);
            char *value = string_duplicate_len(first_equal + 1, value_length); 
            
            config_t config_entry = {.name = name};
            
            if(strcmp(value, "true") == 0){
                config_entry.type = BOOLEAN;
                config_entry.value.boolean = true;
            }else if(strcmp(value, "false") == 0){
                config_entry.type = BOOLEAN;
                config_entry.value.boolean = false;
            }else if(isdigit(value[0])){
                if(strchr(value, '.') == NULL){
                    config_entry.type = INTEGER;
                    config_entry.value.integer = atoi(value);
                }else{
                    config_entry.type = REAL;
                    config_entry.value.real = atof(value);
                } 
            }else{
                config_entry.type = STRING;
                config_entry.value.string = value;
            }
            core_config_set(config_entry);
            free(name);
            free(value);
        }
    }

    ice_cpu_info cpu_info;
    ice_cpu_get_info(&cpu_info);
    core_log(INFO, "CPU: %s", cpu_info.name);
    core_log(INFO, "%d cores", cpu_info.cores);
    core_config_set((config_t){
        .name = "cpu_cores",
        .type = INTEGER,
        .value = cpu_info.cores,
    }); 

    char *path = string_path(argv[0]); 
    char *mod_directory_path = string_join(2, path, "mods");
    free(path);

    char **mod_names = platform_enumerate_directory(mod_directory_path, false); 

    core_event_register("start");
    core_event_register("loop");
    core_event_register("quit");
       
    core_log(INFO, "mod directory: %s", mod_directory_path);
    for(size_t i = 0; i < list_size(mod_names); i++){
        char *mod_filename = mod_names[i];
        char *mod_path = string_join(3, mod_directory_path, PATH_SEPARATOR_STR, mod_filename);

        core_log(INFO, "loading %s", mod_filename);
        shared_object_t *mod_so = platform_library_load(mod_path);
        free(mod_path);

        if(!mod_so){
            core_log(ERROR, "could not load %s", mod_filename);
            free(mod_filename);
            continue;
        }
        start_func_t start = (start_func_t)platform_library_load_symbol(mod_so, "load");
        if(!start)
            continue;
        

        module_desc_t descriptor = start(&core_interface);
        

        module_t module_entry = {
            .name = descriptor.modid,
            .version = descriptor.version,
            .interface = descriptor.interface,
            .shared_object = mod_so, 
        };
        list_push(modules, module_entry);
    }     
    
    list_free(mod_names);
    free(mod_directory_path);


    core_event_trigger("start", &core_interface);
    while(1){
        core_event_trigger("loop", &core_interface);
    }

    return 0;
}

void cleanup(void){
    for(size_t i = 0; i < list_size(modules); i++){
        free(modules[i].name);
        platform_library_unload(modules[i].shared_object);
    }
    list_free(modules);
    str_hash_destroy(&module_hashtable);

    for(size_t i = 0; i < list_size(events); i++){
        free(events[i].name);
        list_free(events[i].callbacks);
    }
    list_free(events);
    str_hash_destroy(&event_hashtable);

    for(size_t i = 0; i < list_size(configs); i++){
        free(configs[i].name);
        if(configs[i].type == STRING)
            free(configs[i].value.string);
    }
    list_free(configs);



}

void core_quit(int status){
    core_event_trigger("quit", &core_interface);
    exit(status);
    }

void core_event_register(const char* name){
    event_t event = {
        string_duplicate(name),
        list_init(event_callback_t)
    };
    list_push(events, event);
    str_hash_insert(&event_hashtable, event.name, list_size(events) - 1);
}

void core_event_trigger (const char *name, void *context){
    size_t index = str_hash_lookup(&event_hashtable, name);
    for(size_t i = 0; i < list_size(events[index].callbacks); i++){
        events[index].callbacks[i](context);
    }
}

void core_event_listen(const char *name, event_callback_t callback){
    size_t index = str_hash_lookup(&event_hashtable, name);

    if(index == UINT64_MAX){
        core_log(WARNING, "unknown event: %s", name);
        return;
    }
    list_push(events[index].callbacks, callback);
}

void core_config_set(config_t config){
    config_t copy = (config_t){.name = string_duplicate(config.name), .type = config.type};
    if(config.type == STRING){
        copy.value.string = string_duplicate(config.value.string);
    }else{
        copy.value = config.value;
    }

    size_t index = str_hash_lookup(&config_hashtable, config.name);
    if(index != UINT64_MAX){
        configs[index] = copy; 
        return;
    }

    list_push(configs, copy);
    str_hash_insert(&config_hashtable, copy.name, list_size(configs) - 1);
}

config_t core_config_get(const char *name){
    uint64_t index = str_hash_lookup(&config_hashtable, name);
    
    if(index > list_size(configs)){
        config_t empty = {.name = "empty", .type = EMPTY, .value.integer = 0 };
        return empty;
    }
    return configs[index];
}

void core_require_version(const char *modid, version_t min_version, version_t max_version){
    
}

void core_require(const char *modid){

}

interface_t* core_module_get_interface(const char *modid){

}

char **platform_enumerate_directory(char *directory_path, bool directories) {
    if (directory_path == NULL) {
        return NULL;
    }
    char **file_list = list_init(char *);

#ifdef UNIX
    DIR *directory = opendir(directory_path);

    if (!directory){
        core_log(ERROR, "invalid search handle for path %s", directory_path);    
        return NULL;
    }
    
    struct dirent *entry;

    while ( (entry = readdir(directory)) ) {
        if (directories && entry->d_type != 4) 
            continue;
        if (!directories && entry->d_type != 8)
            continue;
        
        list_push(file_list, string_duplicate(entry->d_name));
    }
    closedir(directory);

#elif defined(WINDOWS) //copied directly out of M$ docs
    char *search_path = string_join(2, directory_path, "\\*");
    WIN32_FIND_DATA ffd;
    HANDLE hFind = INVALID_HANDLE_VALUE;

    hFind = FindFirstFile(search_path, &ffd);
    free(search_path);

    if (INVALID_HANDLE_VALUE == hFind) {
        core_log(ERROR, "invalid search handle for path %s", directory_path);    
        return NULL;
    }

    do{
        if ((bool)(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == directories){
            list_push(file_list, string_duplicate(ffd.cFileName));
        }
    }while (FindNextFile(hFind, &ffd) != 0);

#endif 
    return file_list;
}

shared_object_t *platform_library_load(char *path) {
    #ifdef UNIX
        return dlopen(path, RTLD_LAZY);
    #elif defined(WINDOWS)
        return LoadLibrary(path);
    #endif 
    return NULL;
}

function_pointer_t *platform_library_load_symbol(shared_object_t *object, char *name) {
    function_pointer_t *function = NULL;
    #ifdef UNIX
        function = dlsym(object, name);
    #elif defined(WINDOWS)
        function = (function_pointer_t*)GetProcAddress(object, name);
    #endif 
    
    if(function == NULL){
        core_log(ERROR, "function not found: %s", name);
        return NULL; 
    }
    return function;
}
void platform_library_unload(shared_object_t *object) {
    #ifdef UNIX
        dlclose(object);
    #elif defined(WINDOWS)
        FreeLibrary(object);
    #endif 
}


void core_log_(log_category_t category, char *restrict format, ...){
    FILE *output_file = stderr;
    
    switch(category){
        case VERBOSE:fputs("[\033[34mVERBSE\033[0m]", output_file);break;         
        case INFO:   fputs("[\033[34mINFO\033[0m]\t", output_file);break;         
        case WARNING:fputs("[\033[93mWARN\033[0m]\t", output_file);break;
        case ERROR:  fputs("[\033[31mERROR\033[0m]\t", output_file);break;
        case CRITICAL:fputs("[\033[31mCRITICAL\033[0m]\t", output_file);break;
        case DEBUG:  fputs("[\033[35mDEBUG\033[0m]\t", output_file);break;
    }
    va_list args;
    va_start(args, format);
    vfprintf(output_file, format, args);
    va_end(args);
    fputs("\n", output_file);
}
