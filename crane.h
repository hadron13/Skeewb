#include<stdint.h>
#include<stdbool.h>
#include<stdarg.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<stdlib.h>
#include<assert.h>


#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#   define WINDOWS
#elif defined(__unix__) || defined(__unix)
#   define UNIX
#endif

#ifdef WINDOWS 
#   define PATH_SEP "\\"
#   define EXEC_EXT ".exe"
#   define DYLIB_EXT ".dll"
#   define SILENCE " > nul "
#   define WIN32_MEAN_AND_LEAN
#   include<windows.h>
#   undef CRITICAL
#   undef ERROR
LPSTR GetLastErrorAsString(void);
#elif defined(UNIX)
#   define PATH_SEP "/"
#   define EXEC_EXT ".x86_64"
#   define DYLIB_EXT ".so"
#   define SILENCE " > /dev/null "
#   include<dirent.h>
#   include<unistd.h>
#   include<sys/stat.h>
#   include<errno.h>
#endif

#define S PATH_SEP

typedef struct {
    size_t size, capacity;
} list_header_t;

#define list_header(l)  ((list_header_t *)(l) - 1)

#define list_size(l)    ((l) ? list_header(l)->size : 0)
#define list_capacity(l)((l) ? list_header(l)->capacity : 0)
#define list_full(l)    (list_size(l) == list_capacity(l))
#define list_init(t)    (list__init(sizeof(t)))
#define list_free(l)    (free(list_header(l)))

#define list_push(l, e) (  (l) = (!l)? list_init(*l) : \
                         list_full(l)? list__resize((l), sizeof(*l),list_capacity(l)*2):\
                                        (l),\
                         (l)[list_header(l)->size++] = (e))

#define list_pop(l)     (list_header(l)->size--, (l) = (list_size(l) <= list_capacity(l) / 2)?\
                         list__resize((l), sizeof(*l), list_capacity(l)/2) : (l))

#define list_swap_delete(l, i)((l)[i] = (l)[list_size(l) - 1], list_pop(l))

#define list_join(l1, l2) (l1 = list__join(l1, l2, sizeof(*l1)), assert(sizeof(*(l1))==sizeof(*(l2))) )

#define list_resize(l, s)(l = list__resize(l, sizeof(*(l)), s))

void *list__init(size_t element_size){
    list_header_t *header = malloc(element_size + sizeof(list_header_t));
    if(header == NULL)
        return NULL;
    header->size = 0;
    header->capacity = 1;
    return (void*)(header + 1);
}

void *list__resize(void *list, size_t element_size, size_t new_ammount) {
    list_header(list)->capacity = new_ammount;
    return (list_header_t*)realloc(list_header(list), (element_size * new_ammount) + sizeof(list_header_t)) + 1;
}

void *list__join(void *list1, void *list2, size_t element_size){
    size_t total_size = (list_size(list1) + list_size(list2));

    list1 = list__resize(list1, element_size, total_size);
    memcpy( (unsigned char*)list1 + list_size(list1), list2, list_size(list2) * element_size);

    list_header(list1)->size += list_size(list2);
    return list1;
}

typedef struct{
    size_t size;
    char *cstring;
}string_t;

#define str(cstr) ((string_t){.size = strlen(cstr), .cstring = (cstr)})
#define str_alloc(str_size) ((string_t){.size = (str_size), .cstring = malloc((str_size) + 1)})
#define str_free(str)(free(str.cstring))
#define str_null ((string_t){.size = 0, .cstring = NULL})

typedef string_t* string_arena_t;

string_t str_arena_add(string_arena_t *arena, string_t str){
    list_push(*arena, str);
    return str;
}

void str_arena_free(string_arena_t arena){
    for(size_t i = 0; i < list_size(arena); i++){
        free(arena[i].cstring);
    }
    list_free(arena);
}

#define str_arena_join(arena, ...)(str_arena_add(arena, string_join(__VA_ARGS__)))
#define str_arena_join_sep(arena, separator, ...)(str_arena_add(arena, string_join_sep(separator, __VA_ARGS__)))
#define str_arena_path(arena, ...)(str_arena_add(arena, string_path(__VA_ARGS__)))


typedef enum { DEBUG, VERBOSE, INFO, WARNING, ERROR, CRITICAL } log_category_t;

string_t string_duplicate(string_t string);
string_t string_duplicate_len(string_t string, size_t length);
string_t string_no_ext(string_t filename);
string_t string_get_ext(string_t filename);

string_t string_join_(unsigned int separator, ...);
string_t string_join_varargs(char separator, va_list args);
#define string_join(...)(string_join_(0, __VA_ARGS__, str_null))
#define string_join_sep(separator, ...)(string_join_(separator, __VA_ARGS__, str_null))
#define string_path(...)(string_join_((PATH_SEP)[0], __VA_ARGS__, str_null))


string_t cstring_path_(char *root, ...);
#define path(...)(cstring_path_(__VA_ARGS__, NULL))
#define arena_path(arena, ...)(str_arena_add(arena, cstring_path_(__VA_ARGS__, NULL)))

string_t *enumerate_directory(string_t path, bool list_directories);
bool     make_directory(string_t name);
bool     file_exists(string_t filename);
bool     move(string_t old_path, string_t new_path);
bool     is_file_older_than(string_t path1, string_t path2);
string_t get_compiler();

int compile_(string_t output, string_t flags, ...);
#define compile(output, flags, ...) (compile_(output, flags, __VA_ARGS__, str_null))

int rebuild_(char *source, int argc, char **argv);
#define rebuild(argc, argv)(rebuild_(__FILE__, argc, argv))

void crane_log_(const char *restrict file, size_t line, const char *restrict function, log_category_t category, char *restrict format, ...);
#define crane_log(category, ...) crane_log_(__FILE__, __LINE__, __func__, category,  __VA_ARGS__);


string_t string_duplicate(string_t string){
    string_t new_string = {.size = string.size, .cstring = malloc(string.size + 1) };
    memcpy(new_string.cstring, string.cstring, string.size + 1);
    return new_string;
}

string_t string_duplicate_len(string_t string, size_t length){
    size_t final_length =  string.size < length? string.size : length;
    string_t new_string = {.size = final_length, .cstring = malloc(final_length + 1)};
    memcpy(new_string.cstring, string.cstring, final_length);
    new_string.cstring[final_length] = 0;
    return new_string;
}

string_t string_no_ext(string_t filename){
    char *first_dot = strchr(filename.cstring, '.');
    if(!first_dot)
        return filename;
    
    size_t remaining = first_dot - filename.cstring;

    return string_duplicate_len(filename, remaining);
}

string_t string_get_ext(string_t filename){
    char *first_dot = strchr(filename.cstring, '.');
    return string_duplicate(str(first_dot));
}

string_t string_join_(unsigned int separator, ...){
    va_list args; 
    va_start(args, separator); 

    return string_join_varargs((char)separator, args);
}

string_t string_join_varargs(char separator, va_list args){
    va_list args_copy;
    va_copy(args_copy, args);
    
    size_t total_size = 0;
    
    string_t current_string = va_arg(args, string_t);

    while(current_string.size > 0){
        total_size += current_string.size;
        if(separator)
            total_size++;
        current_string = va_arg(args, string_t);
    }
    if(separator)
        total_size--;

    va_end(args);
    
    string_t joined_string = str_alloc(total_size);
    
    for(size_t current_character = 0; current_character < total_size;){
        if(separator && current_character != 0){
            joined_string.cstring[current_character] = separator;
            current_character++;
        }

        string_t current_string = va_arg(args_copy, string_t);

        memcpy(joined_string.cstring + current_character, current_string.cstring, current_string.size);
        current_character += current_string.size;
    }
    joined_string.cstring[total_size] = 0;

    va_end(args_copy);
    return joined_string;
}

string_t cstring_path_(char *root, ...){
    va_list args;
    va_start(args, root);
    size_t total_size = 0, total_strings = 0;
    
    char *current_string = root;

    while(current_string != NULL){
        total_size += strlen(current_string) + 1;
        total_strings++;
        current_string = va_arg(args, char *);
    }
    total_size--;
    va_end(args);
    va_start(args, root);
    
    string_t joined_string = str_alloc(total_size);
    joined_string.cstring[0] = 0;

    current_string = root; 
    for(size_t current_character = 0; current_character < total_size;){

        current_character += strlen(current_string);
        strcat(joined_string.cstring, current_string);

        if(current_character < total_size - 1){
            joined_string.cstring[current_character] = PATH_SEP[0];
            current_character++;
        } 
        current_string = va_arg(args, char*);
    }
    va_end(args);
    return joined_string;
}

string_t *enumerate_directory(string_t path, bool list_directories){
    if (path.cstring == NULL) {
        return NULL;
    }
    string_t *file_list = list_init(string_t);
    
#ifdef UNIX
    DIR *directory = opendir(path.cstring);

    if (!directory){
        crane_log(ERROR, "invalid search handle for path %s", path.cstring);    
        return NULL;
    }
    struct dirent *entry;

    while ( (entry = readdir(directory)) ) {
        if (list_directories && entry->d_type != 4 || strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) 
            continue;
        if (!list_directories && entry->d_type != 8)
            continue;
        
        list_push(file_list, string_duplicate(str(entry->d_name)));
    }
    closedir(directory);

#elif defined(WINDOWS) //copied directly out of M$ docs
    string_t search_path = string_join(path, "\\*");
    WIN32_FIND_DATA ffd;
    HANDLE hFind = INVALID_HANDLE_VALUE;

    hFind = FindFirstFile(search_path.cstring, &ffd);
    str_free(search_path);

    if (INVALID_HANDLE_VALUE == hFind) {
        crane_log(ERROR, "invalid search handle for path %s", path.cstring);    
        return NULL;
    }

    do{
        if ((bool)(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == list_directories){
            list_push(file_list, string_duplicate(str(ffd.cFileName)));
        }
    }while (FindNextFile(hFind, &ffd) != 0);

#endif 
    return file_list;    
}

bool make_directory(string_t name){ 
    crane_log(INFO, "creating directory %s", name.cstring);
#   ifdef UNIX
        if (mkdir(name.cstring, 0755) < 0) {
            if (errno == EEXIST) {
                errno = 0;
            } else {
                crane_log(CRITICAL, "could not create directory %s: %s", name, strerror(errno));
                return false;  
            }
        }
#   elif defined(WINDOWS)
    if (!CreateDirectoryA(name.cstring, NULL)) {
        //crane_log(WARNING, "could not create directory %s: %s", name, GetLastErrorAsString());
        return false;
    }
#endif 
    return true;
}


bool file_exists(string_t filename){
    FILE *file = fopen(filename.cstring, "r");
    if(!file)
        return false;

    fclose(file);
    return true;
}


bool move(string_t old_path, string_t new_path){
    #ifdef UNIX
    int status = rename(old_path.cstring, new_path.cstring);
    if(status){
        crane_log(ERROR, "%s", strerror(errno));
    }
    return status;
    #elif defined(WINDOWS)
    
    #endif
}


int compile_(string_t output, string_t flags, ...){
    va_list args;
    va_start(args, flags);
    
    bool should_compile = !file_exists(output);

    for(string_t source_file = va_arg(args, string_t); !should_compile && source_file.size > 0;){
        
        if(is_file_older_than(output, source_file)){
            should_compile = true;
        }
        source_file = va_arg(args, string_t);
    }

    if(!should_compile)
        return 0;

    va_end(args);
    va_start(args, flags);

    string_t dash_o = str("-o");

    string_t sources = string_join_varargs(' ', args);
    string_t command = string_join_sep(' ', get_compiler(), sources, flags, str("-o"), output);

    crane_log(VERBOSE, "%s", command.cstring)
    int status = system(command.cstring);
    
    str_free(sources);
    str_free(command);

    if(status){
        crane_log(ERROR, "compiled with code %d", status);
    }

    return status;
}

int rebuild_(char *source, int argc, char **argv){
    
#   ifdef WINDOWS 
    return 0;
#   elif defined(UNIX)
    string_t executable = str(argv[0]);
    string_t source_file = str(source);

    if(is_file_older_than(executable, source_file)){
        crane_log(INFO, "rebuilding...");
        compile(executable, str("-g"), source_file);
        system(executable.cstring);
        exit(0);
    }
    return 0;
#endif

}

bool is_file_older_than(string_t path1, string_t path2){
#ifdef WINDOWS 
    FILETIME path1_time, path2_time;

    SECURITY_ATTRIBUTES saAttr = {0};
    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    saAttr.bInheritHandle = TRUE;

    HANDLE path1_fd = CreateFile(path1.cstring, GENERIC_READ, FILE_SHARE_READ, &saAttr, OPEN_EXISTING, FILE_ATTRIBUTE_READONLY, NULL);
    if (path1_fd == INVALID_HANDLE_VALUE) {
        crane_log(CRITICAL, "could not open file %s: %s", path1.cstring, GetLastErrorAsString());
    }
    if (!GetFileTime(path1_fd, NULL, NULL, &path1_time)) {
        crane_log(CRITICAL, "could not get time of %s: %s", path1.cstring, GetLastErrorAsString());
    }
    CloseHandle(path1_fd);

    HANDLE path2_fd = CreateFile(path2.cstring, GENERIC_READ, FILE_SHARE_READ, &saAttr, OPEN_EXISTING, FILE_ATTRIBUTE_READONLY, NULL);
    if (path2_fd == INVALID_HANDLE_VALUE) {
        crane_log(CRITICAL, "could not open file %s: %s", path2.cstring, GetLastErrorAsString());
    }
    if (!GetFileTime(path2_fd, NULL, NULL, &path2_time)) {
        crane_log(CRITICAL, "could not get time of %s: %s", path2.cstring, GetLastErrorAsString());
    }
    CloseHandle(path2_fd);

    return CompareFileTime(&path1_time, &path2_time) == -1;
#elif defined(UNIX)
    struct stat statbuf = {0};

    if (stat(path1.cstring, &statbuf) < 0) {
        crane_log(CRITICAL, "could not stat %s: %s\n", path1.cstring, strerror(errno));
    }
    int path1_time = statbuf.st_mtime;

    if (stat(path2.cstring, &statbuf) < 0) {
        crane_log(CRITICAL, "could not stat %s: %s\n", path2.cstring, strerror(errno));
    }
    int path2_time = statbuf.st_mtime;

    return path1_time < path2_time;
#endif
    return false;
}


string_t get_compiler(){
    char *compiler_env = getenv("cc");
    if(compiler_env)
        return str(compiler_env);
#if defined(__GNUC__) && !defined(__clang__)
    return str("gcc");
#elif defined(__clang__)
    return str("clang");
#elif defined(_MSC_VER)
    return str("cl.exe");
#endif
}

void crane_log_(const char *restrict file, size_t line, const char *restrict function, log_category_t category, char *restrict format, ...){
    FILE *output_file = stderr;

    switch(category){
        case VERBOSE:fputs("[\033[34mVERBSE\033[0m]", output_file);break;         
        case INFO:   fputs("[\033[34mINFO\033[0m]\t", output_file);break;         
        case WARNING:fputs("[\033[93mWARN\033[0m]\t", output_file);break;
        case ERROR:  fputs("[\033[31mERROR\033[0m]\t", output_file);break;
        case CRITICAL:fputs("[\033[31mCRTCAL\033[0m]", output_file);break;
        case DEBUG:  fputs("[\033[35mDEBUG\033[0m]\t", output_file);break;
    }

    fprintf(output_file, "[\033[34m%s:\033[35m%lu \033[93m%s()\033[0m] ", file, line, function );

    va_list args;
    va_start(args, format);
    vfprintf(output_file, format, args);
    va_end(args);
    fputs("\n", output_file);
}

#ifdef WINDOWS 
LPSTR GetLastErrorAsString(void){
    // https://stackoverflow.com/questions/1387064/how-to-get-the-error-message-from-the-error-code-returned-by-getlasterror

    DWORD errorMessageId = GetLastError();
    assert(errorMessageId != 0);

    LPSTR messageBuffer = NULL;

    DWORD size =
        FormatMessage(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, // DWORD   dwFlags,
            NULL, // LPCVOID lpSource,
            errorMessageId, // DWORD   dwMessageId,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // DWORD   dwLanguageId,
            (LPSTR) &messageBuffer, // LPTSTR  lpBuffer,
            0, // DWORD   nSize,
            NULL // va_list *Arguments
        );

    return messageBuffer;
}
#endif
