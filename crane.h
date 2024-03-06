#include<stdarg.h>
#include<stdio.h>
#include<stdlib.h>
#include<stdint.h>
#include<stdbool.h>
#include<string.h>

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#   define WINDOWS
#elif defined(__unix__) || defined(__unix)
#   define UNIX
#endif



#ifdef WINDOWS 
#   define PATH_SEP "\\"
#   define EXEC_EXT ".exe"
#   define DYLIB_EXT ".dll"
#   define WIN32_MEAN_AND_LEAN
#   include<windows.h>
#elif defined(UNIX)
#   define PATH_SEP "/"
#   define EXEC_EXT ".x86_64"
#   define DYLIB_EXT ".so"
#   include<dirent.h>
#   include<unistd.h>
#   include<sys/stat.h>
#   include<errno.h>
#endif


// DS lists

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

#define list_join(l1, l2)(l1 = list__join(l1, l2, sizeof(*l1)), assert(sizeof(*(l1))==sizeof(*(l2)))

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

typedef char** string_arena;

char *str_arena_add(string_arena arena, char *str){
    list_push(arena, str);
    return str;
}

void str_arena_free(string_arena arena){
    for(size_t i = 0; i < list_size(arena); i++){
        free(arena[i]);
    }
    list_free(arena);
}






typedef enum { DEBUG, VERBOSE, INFO, WARNING, ERROR, CRITICAL } log_category_t;


void    make_directory(const char *name);
char  **enumerate_directory(const char *path, bool list_directories); // list output
void    set_cwd(const char *path);
char   *get_cwd();
char   *get_compiler();
bool    file_exists(const char *filename);
bool    file1_older_than_file2(char *path1, char *path2);
int     compile(char **sources, char *flags, char *output); // list input
void    rebuild_(char *file, int argc, char **argv);
#define rebuild(argc, argv) rebuild_(__FILE__, argc, argv);

char *string_duplicate(const char *src);
char *string_duplicate_len(const char *src, size_t len);
char *string_directory(const char *filename);
char *string_extension(const char* filename);
char *string_path(size_t number, ...);
char *string_join(size_t number, ...);
char *string_from_list(char **list, char separator);

void crane_log_(const char *restrict file, size_t line, const char *restrict function, log_category_t category, char *restrict format, ...);
#define crane_log(category, ...) crane_log_(__FILE__, __LINE__, __func__, category,  __VA_ARGS__);


void make_directory(const char *name){
    
#   ifdef UNIX
        if (mkdir(name, 0755) < 0) {
            if (errno == EEXIST) {
                errno = 0;
                crane_log(WARNING, "directory %s already exists", name);
            } else {
                crane_log(CRITICAL, "could not create directory %s: %s", name, strerror(errno));
            }
        }
#   elif defined(WINDOWS)
        //TODO: windows
#endif 
}



bool file1_older_than_file2(char *path1, char *path2){

#ifdef WINDOWS 
    FILETIME path1_time, path2_time;

    Fd path1_fd = fd_open_for_read(path1);
    if (!GetFileTime(path1_fd, NULL, NULL, &path1_time)) {
        PANIC("could not get time of %s: %s", path1, GetLastErrorAsString());
    }
    fd_close(path1_fd);

    Fd path2_fd = fd_open_for_read(path2);
    if (!GetFileTime(path2_fd, NULL, NULL, &path2_time)) {
        PANIC("could not get time of %s: %s", path2, GetLastErrorAsString());
    }
    fd_close(path2_fd);

    return CompareFileTime(&path1_time, &path2_time) == 1;
#elif defined(UNIX)
    struct stat statbuf = {0};

    if (stat(path1, &statbuf) < 0) {
        crane_log(CRITICAL, "could not stat %s: %s\n", path1, strerror(errno));
    }
    int path1_time = statbuf.st_mtime;

    if (stat(path2, &statbuf) < 0) {
        crane_log(CRITICAL, "could not stat %s: %s\n", path2, strerror(errno));
    }
    int path2_time = statbuf.st_mtime;

    return path1_time > path2_time;
#endif

}


char  **enumerate_directory(const char *path, bool list_directories){
    if (path == NULL) {
        return NULL;
    }
    char **file_list = list_init(char *);

#ifdef UNIX
    DIR *directory = opendir(path);

    if (!directory){
        crane_log(ERROR, "invalid search handle for path %s", path);    
        return NULL;
    }
    
    struct dirent *entry;

    while ( (entry = readdir(directory)) ) {
        if (list_directories && entry->d_type != 4 || strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) 
            continue;
        if (!list_directories && entry->d_type != 8)
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

bool file_exists(const char *filename){
    FILE *file = fopen(filename, "r");
    if(file){
        fclose(file);
        return true;
    }
    return false;
}

char *get_compiler(){
    char *compiler_env = getenv("compiler");
    if(compiler_env)
        return compiler_env;
#if defined(__GNUC__) && !defined(__clang__)
    return "gcc";
#elif defined(__clang__)
    return "clang";
#elif defined(_MSC_VER)
    return "cl.exe";
#endif
}


int compile(char **sources, char *flags, char *output){
    bool should_compile = !file_exists(output);
    
    if(!should_compile){
        for(size_t i = 0; i < list_size(sources); i++){
            if(file1_older_than_file2(sources[i], output)){
                should_compile = true;
            }
        }
    }
    if(!should_compile)
        return 0;
    char *joined_sources = string_from_list(sources, ' ');
    char *command = string_join(7, get_compiler(), " ", joined_sources, " ", flags, " -o ", output);
    
    crane_log(VERBOSE, "%s", command);
    int status = system(command);

    free(command);
    if(status != 0){
        crane_log(ERROR, "compilation error");
    }
    return status;
}

void rebuild_(char *file, int argc, char **argv){
    
    if(argc < 1)
        return;

    if(file1_older_than_file2(file, argv[0])){
        crane_log(INFO, "rebuilding");
        char *command = string_join(5, get_compiler()," ", file, " -o ", argv[0]);
        if(system(command)){
            crane_log(ERROR, "error rebuilding");
            exit(-1);
        }
        free(command);
        system(argv[0]);        
        exit(0);
    }
}


char *string_duplicate(const char *src){
    char *new_buffer = malloc(strlen(src) + 1);
    if(!new_buffer)
        return NULL;
    strcpy(new_buffer, src);
    return new_buffer;
}

char *string_duplicate_len(const char *src, size_t len){

    size_t string_size = strlen(src);
    size_t new_length = len < string_size ? len : string_size;

    char*result=malloc(new_length+1);
    if(result){
        memcpy(result, src, new_length);
        result[new_length] = 0;
    }
    return result;

}


char *string_directory(const char *filename){
    if(!filename)
        return NULL;
    char *last_slash = strrchr(filename, (PATH_SEP)[0]);
    if(!last_slash)
        return NULL;
    
    size_t path_size = (size_t)(last_slash - filename) + 1;
    
    return string_duplicate_len(filename, path_size);
}

char *string_extension(const char* filename){
    if(!filename)
        return NULL;
    char *last_point= strrchr(filename, '.');
    if(!last_point)
        return (char*)filename;

    return last_point;
}

char *string_path(size_t number, ...){
    if(number <= 0)
        return NULL;

    va_list args; 
    va_start(args, number); 

    size_t total_size = 0;

    for (size_t i = 0; i < number; i++) {
        char *string = va_arg(args, char*);
        if(!string)
            continue;
        
        total_size += strlen(string);
    }

    va_end(args);
 
    char *final_string = malloc(total_size + number);
    if(!final_string)
        return NULL;
    
    final_string[0] = '\0'; 

    va_start(args, number);
   
    for (size_t i = 0; i < number; i++) {
        strcat(final_string, va_arg(args, char*));
        if(i < number - 1)
            strcat(final_string, PATH_SEP);
    }

    va_end(args);

    return final_string;
}

char *string_join(size_t number, ...){
    
    if(number <= 0)
        return NULL;

    va_list args; 
    va_start(args, number); 

    size_t total_size = 0;

    for (size_t i = 0; i < number; i++) {
        char *string = va_arg(args, char*);
        if(!string)
            continue;
        
        total_size += strlen(string);
    }

    va_end(args);
 
    char *final_string = malloc(total_size + 1);
    if(!final_string)
        return NULL;
    
    final_string[0] = '\0'; 

    va_start(args, number);
   
    for (size_t i = 0; i < number; i++) {
        strcat(final_string, va_arg(args, char*)); 
    }

    va_end(args);

    return final_string;
}

char *string_from_list(char **list, char separator){
    size_t total_size = 0;

    for(size_t i = 0; i < list_size(list); i++){
        total_size += strlen(list[i]);
        
        if(separator && i < list_size(list) -1)
            total_size++;
    }
    
    char *final_string = malloc(total_size +  1);

    
    for(size_t i = 0; i < list_size(list); i++){
        strcat(final_string, list[i]); 
        if(separator && i < list_size(list) -1)
            strcat(final_string, &separator);
    }
    return final_string;

}

void crane_log_(const char *restrict file, size_t line, const char *restrict function, log_category_t category, char *restrict format, ...){
    FILE *output_file = stderr;

    switch(category){
        case VERBOSE:fputs("[\033[34mVERBSE\033[0m]", output_file);break;         
        case INFO:   fputs("[\033[34mINFO\033[0m]\t", output_file);break;         
        case WARNING:fputs("[\033[93mWARN\033[0m]\t", output_file);break;
        case ERROR:  fputs("[\033[31mERROR\033[0m]\t", output_file);break;
        case CRITICAL:fputs("[\033[31mCRITICAL\033[0m]\t", output_file);break;
        case DEBUG:  fputs("[\033[35mDEBUG\033[0m]\t", output_file);break;
    }

    fprintf(output_file, "[\033[34m%s:\033[35m%lu \033[93m%s()\033[0m] ", file, line, function );

    va_list args;
    va_start(args, format);
    vfprintf(output_file, format, args);
    va_end(args);
    fputs("\n", output_file);
}
