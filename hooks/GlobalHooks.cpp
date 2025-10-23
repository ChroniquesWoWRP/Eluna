/*
 * Copyright (C) 2010 - 2024 Eluna Lua Engine <https://elunaluaengine.github.io/>
 * This program is free software licensed under GPL version 3
 * Please see the included DOCS/LICENSE.md for more information
 */

#include "Hooks.h"
#include "HookHelpers.h"
#include "LuaEngine.h"
#include "BindingMap.h"
#include "ElunaIncludes.h"
#include "ElunaTemplate.h"

using namespace Hooks;

#define START_HOOK(EVENT) \
    auto key = EventKey<GlobalEvents>(EVENT);\
    if (!GlobalEventBindings->HasBindingsFor(key))\
        return;

#define START_HOOK_WITH_RETVAL(EVENT, RETVAL) \
    auto key = EventKey<GlobalEvents>(EVENT);\
    if (!GlobalEventBindings->HasBindingsFor(key))\
        return RETVAL;

void Eluna::OnCreatureRemove(Creature* creature)
{
    START_HOOK(GLOBAL_EVENT_ON_CREATURE_REMOVE);
    HookPush(creature);
    CallAllFunctions(GlobalEventBindings, key);
}

void Eluna::OnCreatureAdd(Creature* creature)
{
    START_HOOK(GLOBAL_EVENT_ON_CREATURE_ADD);
    HookPush(creature);
    CallAllFunctions(GlobalEventBindings, key);
}
