#include "types.h"
#include <stdlib.h>
#include <string.h>

typedef struct GCData {
    struct GCData* next;
    struct GCData* prev;
} GCData;

GCData* gc_base=NULL;
GCData* gc_prev_top=NULL;
GCData* gc_cur_top=NULL;

#define PTR_TO_GC_DATA(ptr) (((GCData*)ptr)-1)
#define GC_DATA_TO_PTR(ptr) ((void*)(ptr+1))
#define GC_GET_PREV(ptr) ((GCData*)((uintptr_t)(ptr->prev)&~(uintptr_t)1))
#define GC_SET_MARK(ptr) ptr->prev=((GCData*)((uintptr_t)(ptr->prev)|(uintptr_t)1))
#define GC_CLEAR_MARK(ptr) ptr->prev=((GCData*)(((uintptr_t)(ptr->prev)&~(uintptr_t)1)))
#define GC_IS_MARKED(ptr) ((uintptr_t)(ptr->prev)&(uintptr_t)1)

void* gc_alloc(size_t size) {
    GCData* ptr=malloc(size+sizeof(GCData));
    if(!ptr) {
        gc_mark_env();
        gc_mark_new();
        gc_collect(true);
        ptr=malloc(size+sizeof(GCData));
        if(!ptr) return NULL;
    }
    ptr->next=NULL;
    if(gc_cur_top) {
        gc_cur_top->next=ptr;
        ptr->prev=gc_cur_top;
        gc_cur_top=ptr;
    } else {
        ptr->prev=NULL;
        gc_prev_top=NULL;
        gc_cur_top=gc_base=ptr;
    }
    return GC_DATA_TO_PTR(ptr);
}
void* gc_realloc(void* ptr, size_t size) {
    GCData* old_ptr=PTR_TO_GC_DATA(ptr);
    GCData* new_ptr=realloc(old_ptr, size+sizeof(GCData));
    if(!new_ptr) {
        gc_mark_env();
        gc_mark_new();
        gc_collect(true);
        new_ptr=realloc(old_ptr, size+sizeof(GCData));
        if(!new_ptr) return NULL;
    }
    if(new_ptr->prev) {
        GC_GET_PREV(new_ptr)->next=new_ptr;
    }
    if(gc_base==old_ptr) gc_base=new_ptr;
    if(gc_cur_top==old_ptr) gc_cur_top=new_ptr;
    if(gc_prev_top==old_ptr) gc_prev_top=new_ptr;
    return GC_DATA_TO_PTR(new_ptr);
}
void gc_mark(MalValue value) {
    GCData* ptr=NULL;
    switch(value.type) {
        case MAL_TYPE_LIST:
        case MAL_TYPE_VECTOR:
            ptr=PTR_TO_GC_DATA(value.as_list);
            if(!GC_IS_MARKED(ptr)) {
                for(int i=0; i<value.size; i++)
                    gc_mark(value.as_list[i]);
            }
            break;
        case MAL_TYPE_MAP:
            ptr=PTR_TO_GC_DATA(value.as_list);
            if(!GC_IS_MARKED(ptr)) {
                for(int i=1; i<=2*value.size; i++)
                    gc_mark(value.as_list[i]);
            }
            break;

        case MAL_TYPE_SYMBOL:
        case MAL_TYPE_STRING:
        case MAL_TYPE_KEYWORD:
        case MAL_TYPE_ERRMSG:
            ptr=PTR_TO_GC_DATA(value.as_str);
            break;
    }
    if(!value.is_gc || !ptr) return;
    GC_SET_MARK(ptr);
}
void gc_mark_new() {
    if(!gc_prev_top) return;
    GCData* obj=gc_prev_top->next;
    while(obj) {
        GC_SET_MARK(obj);
        obj=obj->next;
    }
}
void gc_collect(bool full) {
    GCData** ptr;
    if(full || !gc_prev_top) {
        ptr=&gc_base;
    } else {
        ptr=&gc_prev_top->next;
    }
    GCData* obj=*ptr;
    gc_cur_top=gc_prev_top;
    while(obj) {
        if(GC_IS_MARKED(obj)) {
            gc_cur_top=obj;
            ptr=&obj->next;
            obj=obj->next;
        } else {
            *ptr=obj->next;
            free(obj);
            obj=*ptr;
        }
    }
    obj=gc_base;
    while(obj) {
        GC_CLEAR_MARK(obj);
        obj=obj->next;
    }
    gc_prev_top=gc_cur_top;
}
void gc_destroy() {
    GCData* ptr=gc_base;
    while(ptr) {
        GCData* new_ptr=ptr->next;
        free(ptr);
        ptr=new_ptr;
    }
    gc_cur_top=gc_prev_top=gc_base=NULL;
}
MalValue mal_copy(MalValue value) {
    return value;
}

MalValue make_list(uint8_t type, MalValue* list, uint32_t size) {
    return (MalValue){
        .type=type,
        .is_gc=1,
        .size=size,
        .as_list=list
    };
}
MalValue make_map(uint8_t type, MalValue* map) {
    return (MalValue){
        .type=type,
        .is_gc=1,
        .size=map[0].capacity,
        .as_list=map
    };
}
MalValue make_const_atomic(uint8_t type, char* string, uint32_t size) {
    return (MalValue){
        .type=type,
        .is_gc=0,
        .length=size,
        .hash=map_hash(string, size),
        .as_str=string
    };
}
MalValue make_number(long data) {
    return (MalValue){
        .type=MAL_TYPE_NUMBER,
        .is_gc=0,
        .as_int=data
    };
}
MalValue make_native_function(MalNativeData* data) {
    return (MalValue){
        .type=MAL_TYPE_NATIVE_FUNCTION,
        .is_gc=0,
        .as_native=data
    };
};
MalValue make_errmsg(char* msg) {
    return (MalValue){
        .type=MAL_TYPE_ERRMSG,
        .is_gc=0,
        .length=(msg ? strlen(msg) : 0),
        .as_str=msg
    };
}

bool string_equals(MalValue a, MalValue b) {
    switch(a.type) {
        case MAL_TYPE_SYMBOL:
        case MAL_TYPE_KEYWORD:
        case MAL_TYPE_STRING:
            if(a.type!=b.type) return false;
            if(a.hash!=b.hash) return false;
            if(a.length!=b.length) return false;
            return strncmp(a.as_str, b.as_str, a.length)==0;
        default:
            return false;
    }
}


// 32-bit FNV-1a hash algorithm;
uint32_t map_hash(const char* string, uint32_t size) {
    const uint32_t fnv_prime=0x01000193;
    const uint32_t fnv_offset_basis=0x811c9dc5;
    uint32_t hash=fnv_offset_basis;
    for(uint32_t i=0; i<size; i++) {
        hash ^= (uint32_t)string[i];
        hash *= fnv_prime;
    }
    return hash;
}
MalValue* map_init(uint32_t size) {
    size_t capacity=size*4/3;
    if(capacity>size) capacity=size;
    MalValue* map=gc_alloc((2*capacity+1)*sizeof(MalValue));
    for(int i=0; i<2*capacity+1; i++)
        map[i]=(MalValue){0};
    map[0].capacity=capacity;
    return map;
}
MalValue* map_set(MalValue* map, MalValue key, MalValue value) {
    MalValue* ptr=map_get(map, key);
    if(ptr) {
        *ptr=value;
        return map;
    }
    MalValue* new_map=map;
    size_t capacity=map[0].capacity;
    if((map[0].size+1)*4/3>capacity && capacity<=0x7FFFFFFF) {
        new_map=map_init(2*capacity);
        for(size_t i=1; i<=capacity; i++) {
            if(map[i].type)
                new_map=map_set(new_map, map[i], map[i+capacity]);
        }
        capacity=new_map[0].capacity;
    }
    for(size_t i=0; i<capacity; i++) {
        size_t index=((key.hash+i)%capacity)+1;
        if(!new_map[index].type) {
            new_map[0].size++;
            new_map[index]=key;
            new_map[index+capacity]=value;
            return new_map;
        }
    }
    return new_map;
}
MalValue* map_get(MalValue* map, MalValue key) {
    uint32_t capacity=map[0].capacity;
    for(size_t i=0; i<capacity; i++) {
        size_t index=((key.hash+i)%capacity)+1;
        if(!map[index].type)
            return NULL;
        if(string_equals(map[index], key))
            return map+index+capacity;
    }
    return NULL;
}

