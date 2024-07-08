/*
 *  Copyright © 2024 hadron13
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy of this software
 *  and associated documentation files (the “Software”), to deal in the Software without
 *  restriction, including without limitation the rights to use, copy, modify, merge, publish,
 *  distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the
 *  Software is furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in all copies or
 *  substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
 *  BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 *  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 *  DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 *  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef SKEEWB_H
#define SKEEWB_H
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#if !defined(WINDOWS) && !defined(UNIX)
#   if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#       define WINDOWS
#   elif defined(__unix__) || defined(__unix)
#       define UNIX
#   endif
#endif



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

struct module_t;
struct event_t;

#include "ds.h"


typedef void(*event_callback_t)(void *context);

typedef enum { CRITICAL, ERROR, WARNING, INFO, VERBOSE, DEBUG} log_category_t;

typedef enum { EMPTY, TYPE_BOOLEAN, TYPE_INTEGER, TYPE_REAL, TYPE_STRING } config_type_t;

typedef struct{
    uint16_t major;
    uint16_t minor;
    uint16_t patch;
}version_t;

typedef struct{
    string_t     modid;
    version_t    version;
    interface_t *interface;
}module_desc_t;

typedef union{
    bool     boolean;
    int64_t  integer;
    double   real;
    string_t string;
}config_value_t;

typedef struct{
    string_t name;
    config_type_t type;
    config_value_t value;
}config_t;

typedef struct{
    string_t name;
    string_t path;
    FILE *file;
}resource_t;


typedef struct{
    version_t version;
    union{
        void         (*console_log)(log_category_t category, char *restrict format, ...);   // printf-like logging function
        void         (*console_log_)(log_category_t category, char *restrict format, ...);  // macro-less console_log
    };
    void             (*event_register)(const string_t name);                                // registers an event
    void             (*event_trigger) (const string_t name, void *context);                 // triggers an event, calling all listening functions
    void             (*event_listen)  (const string_t name, event_callback_t callback);     // register functions to listen to an event
    void             (*quit)(int status);                                                   // engine shutdown, calls 'quit' event
    void             (*config_set)(config_t config);                                        // sets a configuration
    config_t         (*config_get)(const string_t name);                                    // retrieves a configuration
    resource_t      *(*resource_load)(const string_t name, const string_t path);            // loads a new resource, if already loaded, retrieves resource
    resource_t      *(*resource_overload)(const string_t name, const string_t new_path);    // overloads existing resource
    string_t         (*resource_string)(resource_t *resource);                               // reads resource file into a string
    version_t        (*module_get_version)(string_t modid);
    interface_t     *(*module_get_interface)(string_t modid);
    function_pointer_t(*module_get_function)(string_t modid, string_t name);
    void             (*module_reload)(string_t modid);
}core_interface_t;




#ifndef MODULE 
#   define console_log(cat, fmt, ...) console_log(cat, "[\033[34m" __FILE__ ":\033[35m%d \033[93m%s()\033[0m] "fmt, __LINE__, __func__ __VA_OPT__(,) __VA_ARGS__)
#else
#   define console_log(cat, fmt, ...) console_log(cat, "[\033[34m" MODULE" | "__FILE__ ":\033[35m%d \033[93m%s()\033[0m] "fmt, __LINE__, __func__ __VA_OPT__(,) __VA_ARGS__)
#endif
module_desc_t load(core_interface_t *core);

#endif
