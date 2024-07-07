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
#include <stdint.h>
#include <stdbool.h>

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#define WINDOWS
#elif defined(__unix__) || defined(__unix)
#define UNIX
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

#ifndef CORE
typedef void module_t;
typedef void event_t;
#endif

#include "ds.h"


typedef void(*event_callback_t)(void *context);

typedef enum { DEBUG, VERBOSE, INFO, WARNING, ERROR, CRITICAL } log_category_t;

typedef enum { EMPTY, BOOLEAN, INTEGER, REAL, STRING } config_type_t;

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
    bool      boolean;
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
    
}resource_t;


typedef struct{
    version_t version;
    union{
        void (*_log_)(log_category_t category, char *restrict format, ...);         //use this one to override the log function
        void (*console_log)(log_category_t category, char *restrict format, ...);   //for LSP enjoyement
    };
    void         (*event_register)(const string_t name);
    void         (*event_trigger) (const string_t name, void *context);
    void         (*event_listen)  (const string_t name, event_callback_t callback);
    void         (*quit)(int status);
    void         (*config_set)(config_t config);
    config_t     (*config_get)(const string_t name);
    unsigned char* (*load_asset)(const string_t filename);
}core_interface_t;


#ifndef MODULE 
    #define MODULE ""
#endif

#define console_log(cat, fmt, ...) console_log(cat, "[\033[34m" MODULE" |"__FILE__ ":\033[35m%d \033[93m%s()\033[0m] "fmt, __LINE__, __func__ __VA_OPT__(,) __VA_ARGS__)

module_desc_t load(core_interface_t *core);

#endif
