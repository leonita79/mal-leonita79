#include "types.h"
#include "env.h"
#include <string.h>

MalEnv env_init(MalEnv parent, uint32_t size) {
    MalEnv env=map_init(size);
    env[0].as_list=parent;
    return env;
}
MalEnv env_set(MalEnv env, MalValue key, MalValue value) {
    return map_set(env, key, value);
}
MalEnv env_find(MalEnv env, MalValue key) {
    if(!env) return NULL;
    if(map_get(env, key)) return env;
    return env_find(env[0].as_list, key);
}
MalValue env_get(MalEnv env, MalValue key) {
    if(!env)
        return make_errmsg_f("'%.*s' not found", key.length, key.as_str);
    MalValue* value=map_get(env, key);
    if(value) return *value;
    return env_get(env[0].as_list, key);
}
MalEnv env_set_native(MalEnv env, MalNativeData* fn) {
    MalValue key=make_atomic(MAL_TYPE_SYMBOL, fn->name, strlen(fn->name), MAL_GC_CONST);
    MalValue value=(MalValue){
        .type=MAL_TYPE_NATIVE_FUNCTION,
        .is_gc=0,
        .as_native=fn
    };
    return map_set(env, key, value);
}
MalEnv env_set_special(MalEnv env, MalSpecialData* fn) {
    MalValue key=make_atomic(MAL_TYPE_SYMBOL, fn->name, strlen(fn->name), MAL_GC_CONST);
    MalValue value=(MalValue){
        .type=MAL_TYPE_SPECIAL_FORM,
        .is_gc=0,
        .as_special=fn
    };
    return map_set(env, key, value);
}
