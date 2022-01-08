#ifndef ENV_H
#define ENV_H
#include "types.h"
    MalEnv env_init(MalEnv parent, uint32_t size);
MalEnv env_set(MalEnv env, MalValue key, MalValue value);
MalEnv env_find(MalEnv env, MalValue key);
MalValue env_get(MalEnv env, MalValue key);
MalEnv env_set_native(MalEnv env, MalNativeData* fn);
MalEnv env_set_special(MalEnv env, MalSpecialData* fn);
#endif //ENV_H

