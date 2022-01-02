#include "types.h"
#include <stdlib.h>
#include <string.h>

void* gc_base=NULL;
void* gc_prev_top=NULL;
void* gc_cur_top=NULL;

void* gc_alloc(size_t size) {
    void** ptr=malloc(size+2*sizeof(void*));
    if(!ptr) {
        //todo: collect and retry
        return NULL;
    }
    *ptr=NULL;
    if(gc_cur_top) {
        *((void**)gc_cur_top)=ptr;
        ptr[1]=gc_cur_top;
        gc_cur_top=ptr;
    } else {
        ptr[1]=NULL;
        gc_cur_top=gc_prev_top=gc_base=ptr;
    }
    return ptr+2;
}
void* gc_realloc(void* ptr, size_t size) {
    void** old_ptr=(void**)ptr-2;
    void** new_ptr=realloc(old_ptr, size+2*sizeof(void*));
    if(!new_ptr) {
        //todo: collect and retry
        return NULL;
    }
    if(new_ptr[1]) {
        *(void**)new_ptr[1]=new_ptr;
    } else {
        if(gc_base==old_ptr) gc_base=new_ptr;
        if(gc_cur_top==old_ptr) gc_cur_top=new_ptr;
        if(gc_prev_top==old_ptr) gc_prev_top=new_ptr;
    }
    return new_ptr+2;
}
void gc_mark(MalValue value) {
    void* ptr=NULL;
    switch(value.type) {
        case MAL_TYPE_LIST:
        case MAL_TYPE_VECTOR:
        case MAL_TYPE_MAP:
            ptr=value.as_list;
            for(int i=0; i<value.size; i++)
                gc_mark(value.as_list[i]);
            break;

        case MAL_TYPE_SYMBOL:
        case MAL_TYPE_STRING:
        case MAL_TYPE_KEYWORD:
        case MAL_TYPE_ERRMSG:
            ptr=value.as_str;
            break;
    }
    if(!value.is_gc || !ptr) return;
    ((void**)ptr)[-1] = (void*)((uintptr_t)(((void**)ptr)[-1]) | (uintptr_t)1);
}
void gc_collect(bool full) {
    void** ptr=&gc_base;
    void** obj=gc_base;
    while(obj) {
        uintptr_t obj_val=(uintptr_t)obj;
        if(obj_val & (uintptr_t)1) {
            *obj = (void*)(obj_val & ~(uintptr_t)1);
            gc_cur_top=ptr=obj;
            obj=*obj;
        } else {
            *ptr=*obj;
            free(obj);
            obj=*ptr;
        }
    }
}
void gc_destroy() {
    void** ptr=(void**)gc_base;
    while(ptr) {
        void** new_ptr=(void**)((uintptr_t)*ptr & ~(uintptr_t)1);
        free(ptr);
        ptr=new_ptr;
    }
    gc_cur_top=gc_prev_top=gc_base=NULL;
}
MalValue mal_copy(MalValue value) {
    return value;
}

MalValue make_list(uint8_t type, MalValue* list, uint32_t size) {
    MalValue value=(MalValue){.type=type, .is_gc=1, .size=size};
    value.as_list=list;
    return value;
}
MalValue make_const_atomic(uint8_t type, char* string, uint32_t size) {
    MalValue value=(MalValue){.type=type, .is_gc=0, .size=size};
    value.as_str=string;
    return value;
}
MalValue make_number(long data) {
    MalValue value=(MalValue){.type=MAL_TYPE_NUMBER, .is_gc=0};
    value.as_int=data;
    return value;
}
MalValue make_errmsg(char* msg) {
    MalValue value=(MalValue){.type=MAL_TYPE_ERRMSG, .is_gc=0, .size=msg ? strlen(msg) : 0};
    value.as_str=msg;
    return value;
}

