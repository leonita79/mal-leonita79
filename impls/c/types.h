#ifndef TYPES_H
#define TYPES_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

enum {
    MAL_TYPE_NIL,
    MAL_TYPE_FALSE,
    MAL_TYPE_TRUE,
    MAL_TYPE_LIST,
    MAL_TYPE_VECTOR,
    MAL_TYPE_MAP,
    MAL_TYPE_SYMBOL,
    MAL_TYPE_STRING,
    MAL_TYPE_KEYWORD,
    MAL_TYPE_NUMBER,
    MAL_TYPE_NATIVE_FUNCTION,
    MAL_TYPE_SPECIAL_FORM,
    MAL_TYPE_ERRMSG
};

enum {
    MAL_GC_CONST,
    MAL_GC_MEM,
    MAL_GC_TEMP
};

struct MalNativeData;
struct MalSpecialForm;
typedef struct MalValue {
    union {
        struct {
            uint8_t type;
            uint8_t is_gc;
            uint16_t length;
        };
        uint32_t capacity;
    };
    union {
        uint32_t size;
        uint32_t hash;
    };
    union {
        char* as_str;
        struct MalValue* as_list;
        long as_int;
        struct MalNativeData* as_native;
        struct MalSpecialData* as_special;
    };
} MalValue;

typedef MalValue* MalEnv;

typedef MalValue (*MalNativeFn)(uint32_t size, MalValue* args);
typedef struct MalNativeData {
    char* name;
    MalNativeFn fn;
} MalNativeData;
typedef bool (*MalSpecialForm)(MalValue* ast, MalEnv* env);
typedef struct MalSpecialData {
    char* name;
    MalSpecialForm fn;
} MalSpecialData;



void* gc_alloc(size_t size);
void* gc_realloc(void* ptr, size_t size);
void gc_mark(MalValue* value);
void gc_mark_env(MalEnv env);
void gc_mark_new();
void gc_collect();
void gc_destroy();

MalValue make_list(uint8_t type, MalValue* data, uint32_t size);
MalValue make_map(uint8_t type, MalValue* data);
MalValue make_atomic(uint8_t type, char* string, uint32_t size, uint8_t gc);
MalValue make_number(long data);
MalValue make_errmsg(char* msg);
MalValue make_errmsg_f(const char* fmt,...);

bool string_equals(MalValue a, MalValue b);

uint32_t map_hash(const char* string, uint32_t size);
MalValue* map_init(uint32_t size);
MalValue* map_set(MalValue* map, MalValue key, MalValue value);
MalValue* map_get(MalValue* map, MalValue key);
#endif //TYPES_H

