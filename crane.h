
#define _CRT_SECURE_NO_WARNINGS
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
#else
#   error "invalid platform"
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
#   define EXEC_EXT ""
#   define DYLIB_EXT ".so"
#   define SILENCE " > /dev/null "
#   include<dirent.h>
#   include<unistd.h>
#   include<sys/stat.h>
#   include<errno.h>
#endif

#define _ PATH_SEP

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


typedef enum { DEBUG, VERBOSE, INFO, WARNING, ERROR, CRITICAL } log_category_t;


typedef struct{
    size_t length;
    char  *cstr;
}string_t;

#define str(cstring) ((string_t){.length = strlen(cstring), .cstr = (cstring)})
#define str_null     ((string_t){.length = 0, .cstr = NULL})
#define str_alloc(str_size) ((string_t){.length = (str_size), .cstr = malloc((str_size) + 1)})
#define str_free(string) (free((string).cstr))

#define string_join(...)  (string_join_(str(""), __VA_ARGS__, str_null))
#define string_path(...)  (string_join_(str(PATH_SEPARATOR_STR),__VA_ARGS__, str_null))
#define string_join_sep(separator, ...)  (string_join_((separator),__VA_ARGS__, str_null))

string_t string_join_(string_t separator, ...);
string_t string_dup(string_t string);
string_t string_dup_len(string_t string, size_t length);
bool     string_equal(string_t a, string_t b);
string_t string_get_ext(string_t filename);
string_t string_get_path(string_t filename);

typedef string_t* string_temp_t;

string_t str_temp(string_temp_t *temp, string_t string);
void     str_temp_free(string_temp_t temp);
#define  str_temp_join(temp, ...) (str_temp(temp, string_join_(str(""),__VA_ARGS__, str_null)))
#define  temp_path(temp, ...) (str_temp(temp, string_join_(str(PATH_SEP), __VA_ARGS__ , str_null)))




string_t string_join_varargs(string_t separator, va_list args){
    va_list args_copy;
    va_copy(args_copy, args);
 
    size_t total_size = 0;
    string_t current_string = va_arg(args, string_t);

    for(;current_string.cstr != NULL; current_string = va_arg(args, string_t)){
        total_size += current_string.length + separator.length; 
    }
    total_size -= separator.length;

    string_t joined_string = str_alloc(total_size);
    va_end(args);
    
    for(size_t current_character = 0; current_character < total_size;){
        if(current_character != 0){
            memcpy(joined_string.cstr + current_character, separator.cstr, separator.length);
            current_character += separator.length;
        }
        string_t current_string = va_arg(args_copy, string_t);

        memcpy(joined_string.cstr + current_character, current_string.cstr, current_string.length);
        current_character += current_string.length;
    }
    joined_string.cstr[total_size] = '\0';
    va_end(args_copy);

    return joined_string;
}

string_t string_join_(string_t separator, ...){
    va_list args;
    va_start(args, separator);

    return string_join_varargs(separator, args);
}

string_t string_dup(string_t string){
    string_t duplicated = str_alloc(string.length);
    memcpy(duplicated.cstr, string.cstr, string.length + 1);
    return duplicated;
}

string_t string_dup_len(string_t string, size_t length){
    string_t duplicated = str_alloc(string.length);
    memcpy(duplicated.cstr, string.cstr, length + 1);
    return duplicated;
}

bool string_equal(string_t a, string_t b){
    return a.length == b.length && memcmp(a.cstr, b.cstr, a.length) == 0;
}


string_t string_get_ext(string_t filename){
    if(!filename.cstr)
        return str_null;
    char *last_point= strrchr(filename.cstr, '.');
    if(!last_point)
        return filename;
    return string_dup(str(last_point));
}


string_t string_get_path(string_t filename){
    if(!filename.cstr)
        return str_null;
    
    char *last_slash = strrchr(filename.cstr, PATH_SEP[0]);
    
    if(!last_slash)
        return filename;
    return string_dup((string_t){
        .cstr = filename.cstr,
        .length = last_slash - filename.cstr
    });
}


string_t str_temp(string_temp_t *arena, string_t str){
    list_push(*arena, str);
    return str;
}

void str_temp_free(string_temp_t temp){
    for(size_t i = 0; i < list_size(temp); i++){
        str_free(temp[i]);
    }
    list_free(temp);
}

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


string_t *enumerate_directory(string_t path, bool list_directories){
    if (path.cstr == NULL) {
        return NULL;
    }
    string_t *file_list = list_init(string_t);
    
#ifdef UNIX
    DIR *directory = opendir(path.cstr);

    if (!directory){
        crane_log(ERROR, "invalid search handle for path %s", path.cstr);    
        return NULL;
    }
    struct dirent *entry;

    while ( (entry = readdir(directory)) ) {
        if (list_directories && entry->d_type != 4 || strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) 
            continue;
        if (!list_directories && entry->d_type != 8)
            continue;
        
        list_push(file_list, string_dup(str(entry->d_name)));
    }
    closedir(directory);

#elif defined(WINDOWS) //copied directly out of M$ docs
    string_t search_path = string_join(path, str("\\*"));
    WIN32_FIND_DATA ffd;
    HANDLE hFind = INVALID_HANDLE_VALUE;

    hFind = FindFirstFile(search_path.cstr, &ffd);
    str_free(search_path);

    if (INVALID_HANDLE_VALUE == hFind) {
        crane_log(ERROR, "invalid search handle for path %s", path.cstr);    
        return NULL;
    }

    do{
        if ((bool)(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == list_directories){
            list_push(file_list, string_dup(str(ffd.cFileName)));
        }
    }while (FindNextFile(hFind, &ffd) != 0);

#endif 
    return file_list;    
}

bool make_directory(string_t name){ 
    crane_log(INFO, "creating directory %s", name.cstr);
#   ifdef UNIX
        if (mkdir(name.cstr, 0755) < 0) {
            if (errno == EEXIST) {
                errno = 0;
            } else {
                crane_log(CRITICAL, "could not create directory %s: %s", name, strerror(errno));
                return false;  
            }
        }
#   elif defined(WINDOWS)
    if (!CreateDirectoryA(name.cstr, NULL)) {
        //crane_log(WARNING, "could not create directory %s: %s", name, GetLastErrorAsString());
        return false;
    }
#endif 
    return true;
}


bool file_exists(string_t filename){
    FILE *file = fopen(filename.cstr, "r");
    if(!file)
        return false;

    fclose(file);
    return true;
}


bool move(string_t old_path, string_t new_path){
    #ifdef UNIX
    crane_log(VERBOSE, "move %s => %s", old_path.cstr, new_path.cstr);

    int status = rename(old_path.cstr, new_path.cstr);
    if(status){
        crane_log(ERROR, " %s", strerror(errno));
    }
    return status;
    #elif defined(WINDOWS)
    
    MoveFileA(old_path.cstr, new_path.cstr);

    #endif
}


bool copy(string_t source, string_t destination){
    #ifdef UNIX
    string_t command = string_join_sep(str(" "), str("cp"), source, destination);
    #elif defined(WINDOWS)

    string_t win_source = string_dup(source);
    for(int i = 0; i < win_source.length; i++) win_source.cstr[i] = (win_source.cstr[i] == '/')? '\\' : win_source.cstr[i];
    
    string_t win_dest = string_dup(source);
    for(int i = 0; i < win_dest.length; i++) win_dest.cstr[i] = (win_dest.cstr[i] == '/')? '\\' : win_dest.cstr[i];

    string_t command = string_join_sep(str(" "), str("copy"), win_source, win_dest);
    #endif


    crane_log(INFO, "copying %s => %s", source.cstr, destination.cstr);
    int status = system(command.cstr);
    if(status){
        crane_log(ERROR, " '%s' to '%s'  %s", strerror(errno));
    }
    str_free(command);
    return status;
}



int compile_(string_t output, string_t flags, ...){
    va_list args;
    va_start(args, flags);
    
    size_t number_of_sources;

    bool should_compile = !file_exists(output);

    for(string_t source_file = va_arg(args, string_t); !should_compile && source_file.length > 0;){
        
        if(is_file_older_than(output, source_file)){
            should_compile = true;
        }
        number_of_sources++;
        source_file = va_arg(args, string_t);
    }
    va_end(args);
    if(!should_compile)
        return 0;

    va_start(args, flags);

    string_t dash_o = str("-o");

    string_t sources = string_join_varargs(str(" "), args);
    string_t command = string_join_sep(str(" "), get_compiler(), sources, flags, str("-o"), output);

    crane_log(VERBOSE, "%s", command.cstr);
    int status = system(command.cstr);
    
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
        string_t command = string_join_sep(str(" "), executable, (argc>1)? str(argv[1]) : str_null);
        exit(system(command.cstr));
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

    HANDLE path1_fd = CreateFile(path1.cstr, GENERIC_READ, FILE_SHARE_READ, &saAttr, OPEN_EXISTING, FILE_ATTRIBUTE_READONLY, NULL);
    if (path1_fd == INVALID_HANDLE_VALUE) {
        crane_log(CRITICAL, "could not open file %s: %s", path1.cstr, GetLastErrorAsString());
    }
    if (!GetFileTime(path1_fd, NULL, NULL, &path1_time)) {
        crane_log(CRITICAL, "could not get time of %s: %s", path1.cstr, GetLastErrorAsString());
    }
    CloseHandle(path1_fd);

    HANDLE path2_fd = CreateFile(path2.cstr, GENERIC_READ, FILE_SHARE_READ, &saAttr, OPEN_EXISTING, FILE_ATTRIBUTE_READONLY, NULL);
    if (path2_fd == INVALID_HANDLE_VALUE) {
        crane_log(CRITICAL, "could not open file %s: %s", path2.cstr, GetLastErrorAsString());
    }
    if (!GetFileTime(path2_fd, NULL, NULL, &path2_time)) {
        crane_log(CRITICAL, "could not get time of %s: %s", path2.cstr, GetLastErrorAsString());
    }
    CloseHandle(path2_fd);

    return CompareFileTime(&path1_time, &path2_time) == -1;
#elif defined(UNIX)
    struct stat statbuf = {0};

    if (stat(path1.cstr, &statbuf) < 0) {
        crane_log(CRITICAL, "could not stat %s: %s\n", path1.cstr, strerror(errno));
    }
    int path1_time = statbuf.st_mtime;

    if (stat(path2.cstr, &statbuf) < 0) {
        crane_log(CRITICAL, "could not stat %s: %s\n", path2.cstr, strerror(errno));
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
