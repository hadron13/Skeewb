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
#define CORE


#define ICE_CPU_IMPL
#include "ice_cpu.h"
#include "ds.h"
#include "skeewb.h"

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
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

//thank you microsoft
#undef interface
#undef ERROR
#define BOOLEAN BOOLEAN_

#endif


/*
 *  ======== \\    // ||===\\  ||==== //===\\ 
 *     ||     \\  //  ||    || ||     ||      
 *     ||      \\//   ||===//  ||===  \\===\\ 
 *     ||       ||    ||       ||           ||
 *     ||       ||    ||       ||==== \\===// 
 */


typedef struct{
    string_t          name;
    event_callback_t *callbacks;
}event_t;

typedef struct{
    string_t         name;
    version_t        version;
    string_t         path;
    shared_object_t *shared_object;
    interface_t     *interface;
}module_t;

typedef struct{
    char *dependent;
    char *modid;
    version_t min;
    version_t max;
}module_requirement_t;


typedef module_desc_t(*start_func_t)(core_interface_t *interface);

/*
 * ====== ||    || ||\  ||  //===\\ ======== ======  //===\\  ||\  ||  //===\\ 
 * ||     ||    || ||\\ || ||          ||      ||   ||     || ||\\ || ||      
 * ||===  ||    || || \\|| ||          ||      ||   ||     || || \\||  \\===\\ 
 * ||     ||    || ||  \\| ||          ||      ||   ||     || ||  \\|        ||
 * ||      \\==//  ||   ||  \\===//    ||    ======  \\===//  ||   \|  \\===//
 */

//  ====== EVENTS ====== 
void     core_event_register(const string_t name);
void     core_event_trigger (const string_t name, void *context);
void     core_event_listen  (const string_t name, event_callback_t callback);

//  ====== CONFIGS =====
void     core_config_set(config_t config);
config_t core_config_get(const string_t name);

//  ====== MODULES =====


//  ===== LOGGING ======
void core_log_(log_category_t category, char *restrict format, ...);
#define core_log(category, format, ...) core_log_(category, "[\033[34m" MODULE" |"__FILE__ ":\033[35m%d \033[93m%s()\033[0m] "format, __LINE__, __func__ __VA_OPT__(,) __VA_ARGS__)


//  ======= MISC =======
void    parse_argument(char *arg);
bool    version_valid(version_t version, version_t min, version_t max);
char   *core_get_cwd();


//  ===== PLATFORM =====
string_t             *platform_enumerate_directory(string_t directory_path, bool directories); // returns dynamic array of strings
shared_object_t      *platform_library_load(string_t path);
function_pointer_t   *platform_library_load_symbol(shared_object_t *object, string_t name);
void                  platform_library_unload(shared_object_t *object);
void                  platform_print_error();


//  ===== LIFETIME =====
void cleanup(void);
void core_quit(int status);

/*
 *  //===\\  ||      //===\\  ||==\\     /\     ||      //===\\ 
 * ||        ||     ||     || ||   ||   //\\    ||     ||      
 * ||  ===\\ ||     ||     || ||==\\   //  \\   ||      \\===\\ 
 * ||     || ||     ||     || ||   || //====\\  ||            ||
 *  \\===//  ======  \\===//  ||==// //      \\ ======  \\===//
 */

core_interface_t core_interface = {
    {0, 0, 1},
    ._log_ = core_log_,
    .event_register = core_event_register,
    .event_trigger = core_event_trigger,
    .event_listen = core_event_listen,
    .quit = core_quit,
    .config_set = core_config_set,
    .config_get = core_config_get,
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
    

    FILE *config_file = fopen("config.txt", "r");
    if(!config_file){
        core_log(INFO, "reading config file");
        // TODO: read config file
    }

    #ifdef WINDOWS
    if(SetDllDirectoryA("D:\\Skeewb\\build\\mods\\libs") == 0){
        core_log(ERROR, "could not add dll directory with error %i", GetLastError());
    }
    #endif


    if(argc > 1){
        core_log(INFO, "parsing arguments");
        for(size_t i = 1; i < argc; i++){
            char *current_arg = argv[i];

            if(current_arg[0] != '-' || current_arg[1] == '\0'){
                core_log(ERROR, "invalid argument: %s", current_arg);
                continue;
            }
            parse_argument(argv[i] + 1);
        }
    }

    ice_cpu_info cpu_info;
    ice_cpu_get_info(&cpu_info);
    core_log(INFO, "CPU: %s", cpu_info.name);
    core_log(INFO, "%d cores", cpu_info.cores);
    
    core_config_set((config_t){
        .name = str("cpu_cores"),
        .type = INTEGER,
        .value = cpu_info.cores,
    }); 
    
    core_event_register(str("start"));
    core_event_register(str("loop"));
    core_event_register(str("quit"));

    string_temp_t temp = list_init(string_t);
    string_t cwd = str_temp(&temp, string_get_path(str(argv[0])));

    string_t mod_directory = str_temp(&temp, string_path( cwd, str("mods")));
    
    core_log(INFO, "mod directory: %s", mod_directory.cstr);
    string_t *mod_names = platform_enumerate_directory(mod_directory, false); 
    
    for(size_t i = 0; i < list_size(mod_names); i++){
        string_t mod_path = str_temp(&temp, string_path(mod_directory, mod_names[i]));

        if(!string_equal(str_temp(&temp, string_get_ext(mod_path)), str(DYLIB_EXTENSION)))
            continue;
        

        core_log(INFO, "loading %s", mod_names[i].cstr);
        shared_object_t *mod_so = platform_library_load(mod_path);

        if(!mod_so){
            core_log(ERROR, "could not load %s", mod_names[i].cstr);
            continue;
        }
        start_func_t start = (start_func_t)platform_library_load_symbol(mod_so, str("load"));
        if(!start)
            continue;

        module_desc_t descriptor = start(&core_interface);

        list_push(modules, ((module_t){
            .name = string_dup(descriptor.modid),
            .version = descriptor.version,
            .interface = descriptor.interface,
            .shared_object = mod_so, 
        }));
    }     
    
    list_free(mod_names);
    str_temp_free(temp);

    core_event_trigger(str("start"), &core_interface);
    while(1){
        core_event_trigger(str("loop"), &core_interface);
    }

    return 0;
}

void cleanup(void){
    for(size_t i = 0; i < list_size(modules); i++){
        str_free(modules[i].name);
        platform_library_unload(modules[i].shared_object);
    }

    for(size_t i = 0; i < list_size(events); i++){
        str_free(events[i].name);
        list_free(events[i].callbacks);
    }
    
    for(size_t i = 0; i < list_size(configs); i++){
        str_free(configs[i].name);
        if(configs[i].type == STRING)
            str_free(configs[i].value.string);
    }
    list_free(modules);
    list_free(events);
    list_free(configs);
    
    str_hash_destroy(&module_hashtable);
    str_hash_destroy(&event_hashtable);
}

void core_quit(int status){
    core_log(INFO, "quitting with status %d...", status);
    core_event_trigger(str("quit"), &core_interface);
    exit(status);
}

// ===== ===== Events ===== =====

void core_event_register(const string_t name){
    event_t event = {
        string_dup(name),
        list_init(event_callback_t)
    };
    list_push(events, event);
    str_hash_insert(&event_hashtable, event.name.cstr, list_size(events) - 1);
}

void core_event_trigger (const string_t name, void *context){
    size_t index = str_hash_lookup(&event_hashtable, name.cstr);
    if(index == STR_HASH_MISSING){
        core_log(WARNING, "unknown event: %s", name.cstr);
        return;
    }
    for(size_t i = 0; i < list_size(events[index].callbacks); i++){
        events[index].callbacks[i](context);
    }
}

void core_event_listen(const string_t name, event_callback_t callback){
    size_t index = str_hash_lookup(&event_hashtable, name.cstr);

    if(index == STR_HASH_MISSING){
        core_log(WARNING, "unknown event: %s", name.cstr);
        return;
    }
    list_push(events[index].callbacks, callback);
}

// ===== ===== Configs ===== =====

void core_config_set(config_t config){

    size_t index = str_hash_lookup(&config_hashtable, config.name.cstr);

    if(index == STR_HASH_MISSING){
        config_t copy = {
            .name = string_dup(config.name),
            .type = config.type,
            .value = config.type == STRING? (config_value_t){.string = string_dup(config.value.string)} : config.value
        };
        list_push(configs, copy);
        str_hash_insert(&config_hashtable, copy.name.cstr, list_size(configs) - 1);
        return;
    }
    if(configs[index].type == STRING){
        str_free(configs[index].value.string);
    }

    configs[index].type = config.type;
    configs[index].value = config.type == STRING? (config_value_t){.string = string_dup(config.value.string)} : config.value;
}

config_t core_config_get(const string_t name){
    uint64_t index = str_hash_lookup(&config_hashtable, name.cstr);
    
    if(index == STR_HASH_MISSING){
        return (config_t){.name = str("empty"), .type = EMPTY, .value.integer = 0 };
    }
    return configs[index];
}



// ===== ===== Miscellaneous ===== =====

void parse_argument(char *arg){
    char *first_equal = strchr(arg, '=');
    if(!first_equal){ 
        core_config_set((config_t){
            .name = str(arg),
            .type = BOOLEAN,
            .value.boolean = true
        });
        return; 
    }   
    size_t name_length = first_equal - arg; 
    size_t value_length = strlen(first_equal) - 1;
    
    if(name_length == 0){
        core_log(ERROR, "in argument \"%s\", no variable name found", arg);
        return; 
    }
    if(value_length == 0){
        core_log(ERROR, "in argument \"%s\", no value found", arg);
        return;
    }
    
    string_t name = string_dup_len(str(arg), name_length);
    string_t value = string_dup_len(str(first_equal + 1), value_length); 
    
    config_t config_entry = {.name = name};
    
    if(string_equal(value, str("true"))){
        config_entry.type = BOOLEAN;
        config_entry.value.boolean = true;
    }else if(string_equal(value, str("false"))){
        config_entry.type = BOOLEAN;
        config_entry.value.boolean = false;
    }else if(isdigit(value.cstr[0])){
        if(strchr(value.cstr, '.') == NULL){
            config_entry.type = INTEGER;
            config_entry.value.integer = atoi(value.cstr);
        }else{
            config_entry.type = REAL;
            config_entry.value.real = atof(value.cstr);
        } 
    }else{
        config_entry.type = STRING;
        config_entry.value.string = value;
    }
    core_config_set(config_entry);
    str_free(name);
    str_free(value);
}

bool version_valid(version_t version, version_t min, version_t max){

    uint64_t version_mask = ((uint64_t)version.major << 32) + ((uint64_t)version.minor << 16) + version.patch; 
    uint64_t min_mask =     ((uint64_t)min.major << 32)     + ((uint64_t)min.minor << 16)     + min.patch;
    uint64_t max_mask =     ((uint64_t)max.major << 32)     + ((uint64_t)max.minor << 16)     + max.patch; 

    if(version_mask < min_mask)
        return false;
    if(version_mask > max_mask)
        return false;

    return true;
}


// ===== ===== Platform Abstraction ===== =====

string_t *platform_enumerate_directory(string_t directory_path, bool directories) {
    if (directory_path.cstr == NULL) {
        return NULL;
    }
    string_t *file_list = list_init(string_t);
    core_log(DEBUG, "%i", list_size(file_list));

#ifdef UNIX
    DIR *directory = opendir(directory_path.cstr);

    if (!directory){
        core_log(ERROR, "invalid search handle for path %s", directory_path);    
        return NULL;
    }
    
    struct dirent *entry;

    while ( (entry = readdir(directory)) ) {
        if(directories != (entry->d_type == DT_DIR))
            continue;
        if(strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;


        list_push(file_list, string_dup(str(entry->d_name)));
    }
    closedir(directory);

#elif defined(WINDOWS) //copied directly out of M$ docs
    char *search_path = string_path(directory_path, str("\\*"));
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
            list_push(file_list, string_dup(str(ffd.cFileName)));
        }
    }while (FindNextFile(hFind, &ffd) != 0);

#endif 
    return file_list;
}

shared_object_t *platform_library_load(string_t path) {
    shared_object_t *obj = NULL;
    #ifdef UNIX    
        obj = dlopen(path.cstr, RTLD_LAZY);
        if(obj == NULL){
            core_log(ERROR, "loaded %s with error %s", path, dlerror());
        }
    #elif defined(WINDOWS)
        obj = LoadLibraryA(path.cstr);
        if(obj == NULL){
            core_log(ERROR, "loaded %s with error %i", path, GetLastError());
        }
    #endif 
    return obj;
}

function_pointer_t *platform_library_load_symbol(shared_object_t *object, string_t name) {
    function_pointer_t *function = NULL;
    #ifdef UNIX
        function = dlsym(object, name.cstr);
    #elif defined(WINDOWS)
        function = (function_pointer_t*)GetProcAddress(object, name.cstr);
    #endif 
    
    if(function == NULL){
        core_log(ERROR, "function not found: %s", name.cstr);
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
        case VERBOSE: fputs("[\033[34mVERBSE\033[0m]", output_file);break;         
        case INFO:    fputs("[\033[34mINFO\033[0m]\t", output_file);break;         
        case WARNING: fputs("[\033[93mWARN\033[0m]\t", output_file);break;
        case ERROR:   fputs("[\033[31mERROR\033[0m]\t", output_file);break;
        case CRITICAL:fputs("[\033[31mCRITICAL\033[0m]\t", output_file);break;
        case DEBUG:   fputs("[\033[35mDEBUG\033[0m]\t", output_file);break;
    }
    va_list args;
    va_start(args, format);
    vfprintf(output_file, format, args);
    va_end(args);
    fputs("\n", output_file);
}
