/*
* Copyright (C) 2010 - 2024 Eluna Lua Engine <https://elunaluaengine.github.io/>
* This program is free software licensed under GPL version 3
* Please see the included DOCS/LICENSE.md for more information
*/

#ifndef GAMEOBJECTMETHODS_H
#define GAMEOBJECTMETHODS_H

/***
 * Inherits all methods from: [Object], [WorldObject]
 */
namespace LuaGameObject
{
    /**
     * Returns 'true' if the [GameObject] can give the specified [Quest]
     *
     * @param uint32 questId : quest entry Id to check
     * @return bool hasQuest
     */
    int HasQuest(Eluna* E, GameObject* go)
    {
        uint32 questId = E->CHECKVAL<uint32>(2);

        E->Push(go->hasQuest(questId));
        return 1;
    }

    /**
     * Returns 'true' if the [GameObject] is spawned
     *
     * @return bool isSpawned
     */
    int IsSpawned(Eluna* E, GameObject* go)
    {
        E->Push(go->isSpawned());
        return 1;
    }

    /**
     * Returns 'true' if the [GameObject] is a transport
     *
     * @return bool isTransport
     */
    int IsTransport(Eluna* E, GameObject* go)
    {
        E->Push(go->IsTransport());
        return 1;
    }

    /**
     * Returns 'true' if the [GameObject] is active
     *
     * @return bool isActive
     */
    int IsActive(Eluna* E, GameObject* go)
    {
        E->Push(go->isActiveObject());
        return 1;
    }

    /**
     * Returns true if the [GameObject] is a destructible, false otherwise.
     *
     * @return bool isDestructible
     */
    int IsDestructible(Eluna* E, GameObject* go)
    {
        E->Push(go->IsDestructibleBuilding());
        return 1;
    }

    /**
     * Returns display ID of the [GameObject]
     *
     * @return uint32 displayId
     */
    int GetDisplayId(Eluna* E, GameObject* go)
    {
        E->Push(go->GetDisplayId());
        return 1;
    }

    /**
     * Returns the state of a [GameObject]
     * Below are client side [GOState]s off of 3.3.5a
     *
     * <pre>
     * enum GOState
     * {
     *     GO_STATE_ACTIVE             = 0,                        // show in world as used and not reset (closed door open)
     *     GO_STATE_READY              = 1,                        // show in world as ready (closed door close)
     *     GO_STATE_ACTIVE_ALTERNATIVE = 2                         // show in world as used in alt way and not reset (closed door open by cannon fire)
     * };
     * </pre>
     *
     * @return [GOState] goState
     */
    int GetGoState(Eluna* E, GameObject* go)
    {
        E->Push(go->GetGoState());
        return 1;
    }

    /**
     * Returns the [LootState] of a [GameObject]
     * Below are [LootState]s off of 3.3.5a
     *
     * <pre>
     * enum LootState
     * {
     *     GO_NOT_READY = 0,
     *     GO_READY,                                               // can be ready but despawned, and then not possible activate until spawn
     *     GO_ACTIVATED,
     *     GO_JUST_DEACTIVATED
     * };
     * </pre>
     *
     * @return [LootState] lootState
     */
    int GetLootState(Eluna* E, GameObject* go)
    {
        E->Push(go->getLootState());
        return 1;
    }

    /**
     * Returns the [Player] that can loot the [GameObject]
     *
     * Not the original looter and may be nil.
     *
     * @return [Player] player
     */
    int GetLootRecipient(Eluna* E, GameObject* go)
    {
        E->Push(go->GetLootRecipient());
        return 1;
    }

    /**
     * Returns the [Group] that can loot the [GameObject]
     *
     * Not the original looter and may be nil.
     *
     * @return [Group] group
     */
    int GetLootRecipientGroup(Eluna* E, GameObject* go)
    {
        E->Push(go->GetLootRecipientGroup());
        return 1;
    }

    /**
     * Returns the guid of the [GameObject] that is used as the ID in the database
     *
     * @return uint32 dbguid
     */
    int GetDBTableGUIDLow(Eluna* E, GameObject* go)
    {
        E->Push(go->GetSpawnId());
        return 1;
    }

    /**
     * Sets the state of a [GameObject]
     *
     * <pre>
     * enum GOState
     * {
     *     GO_STATE_ACTIVE             = 0,                        // show in world as used and not reset (closed door open)
     *     GO_STATE_READY              = 1,                        // show in world as ready (closed door close)
     *     GO_STATE_ACTIVE_ALTERNATIVE = 2                         // show in world as used in alt way and not reset (closed door open by cannon fire)
     * };
     * </pre>
     *
     * @param [GOState] state : all available go states can be seen above
     */
    int SetGoState(Eluna* E, GameObject* go)
    {
        uint32 state = E->CHECKVAL<uint32>(2, 0);

        if (state == 0)
            go->SetGoState(GO_STATE_ACTIVE);
        else if (state == 1)
            go->SetGoState(GO_STATE_READY);
        else if (state == 2)
            go->SetGoState(GO_STATE_DESTROYED);

        return 0;
    }

    /**
     * Sets the [LootState] of a [GameObject]
     * Below are [LootState]s off of 3.3.5a
     *
     * <pre>
     * enum LootState
     * {
     *     GO_NOT_READY = 0,
     *     GO_READY,                                               // can be ready but despawned, and then not possible activate until spawn
     *     GO_ACTIVATED,
     *     GO_JUST_DEACTIVATED
     * };
     * </pre>
     *
     * @param [LootState] state : all available loot states can be seen above
     */
    int SetLootState(Eluna* E, GameObject* go)
    {
        uint32 state = E->CHECKVAL<uint32>(2, 0);

        if (state == 0)
            go->SetLootState(GO_NOT_READY);
        else if (state == 1)
            go->SetLootState(GO_READY);
        else if (state == 2)
            go->SetLootState(GO_ACTIVATED);
        else if (state == 3)
            go->SetLootState(GO_JUST_DEACTIVATED);

        return 0;
    }

    /**
     * Saves [GameObject] to the database
     *
     */
    int SaveToDB(Eluna* /*E*/, GameObject* go)
    {
        go->SaveToDB();
        return 0;
    }

    /**
     * Removes [GameObject] from the world
     *
     * The object is no longer reachable after this and it is not respawned.
     *
     * @param bool deleteFromDB : if true, it will delete the [GameObject] from the database
     */
    int RemoveFromWorld(Eluna* E, GameObject* go)
    {
        bool deldb = E->CHECKVAL<bool>(2, false);

        // cs_gobject.cpp copy paste
        ObjectGuid ownerGuid = go->GetOwnerGUID();
        if (ownerGuid)
        {
            Unit* owner = eObjectAccessor()GetUnit(*go, ownerGuid);
            if (!owner || !ownerGuid.IsPlayer())
                return 0;

            owner->RemoveGameObject(go, false);
        }

        if (deldb)
            GameObject::DeleteFromDB(go->GetSpawnId());

        go->SetRespawnTime(0);
        go->Delete();

        return 0;
    }

    /**
     * Activates a door or a button/lever
     *
     * @param uint32 delay = 0 : cooldown time in seconds to restore the [GameObject] back to normal. 0 for infinite duration
     */
    int UseDoorOrButton(Eluna* E, GameObject* go)
    {
        uint32 delay = E->CHECKVAL<uint32>(2, 0);

        go->UseDoorOrButton(delay);
        return 0;
    }

    /**
     * Despawns a [GameObject]
     *
     * The gameobject may be automatically respawned by the core
     */
    int Despawn(Eluna* /*E*/, GameObject* go)
    {
        go->SetLootState(GO_JUST_DEACTIVATED);
        return 0;
    }

    /**
     * Respawns a [GameObject]
     */
    int Respawn(Eluna* /*E*/, GameObject* go)
    {
        go->Respawn();
        return 0;
    }

    /**
     * Sets the respawn or despawn time for the gameobject.
     *
     * Respawn time is also used as despawn time depending on gameobject settings
     *
     * @param int32 delay = 0 : cooldown time in seconds to respawn or despawn the object. 0 means never
     */
    int SetRespawnTime(Eluna* E, GameObject* go)
    {
        int32 respawn = E->CHECKVAL<int32>(2);

        go->SetRespawnTime(respawn);
        return 0;
    }

    int SetTemporaryPosition(Eluna* E, GameObject* go)
    {
        if (go->GetSpawnId() != 0)
            return luaL_error(E->L, "Method SetTemporaryPosition can only be used on non-persistant gameobject.");

        uint8 i = 2;
        float posX = go->GetPositionX();
        float posY = go->GetPositionY();
        float posZ = go->GetPositionZ();
        float pitch = 0.0f;
        float roll = 0.0f;
        float yaw = go->GetOrientation();

        if (lua_istable(E->L, i)) {
            lua_rawgeti(E->L, i, 1);
            if (!lua_isnil(E->L, -1)) posX = (float)lua_tonumber(E->L, -1);
            lua_pop(E->L, 1);

            lua_rawgeti(E->L, i, 2);
            if (!lua_isnil(E->L, -1)) posY = (float)lua_tonumber(E->L, -1);
            lua_pop(E->L, 1);

            lua_rawgeti(E->L, i, 3);
            if (!lua_isnil(E->L, -1)) posZ = (float)lua_tonumber(E->L, -1);
            lua_pop(E->L, 1);
            i++;
        }

        if (lua_istable(E->L, i)) {
            lua_rawgeti(E->L, i, 1);
            if (!lua_isnil(E->L, -1)) pitch = (float)lua_tonumber(E->L, -1);
            lua_pop(E->L, 1);

            lua_rawgeti(E->L, i, 2);
            if (!lua_isnil(E->L, -1)) roll = (float)lua_tonumber(E->L, -1);
            lua_pop(E->L, 1);

            lua_rawgeti(E->L, i, 3);
            if (!lua_isnil(E->L, -1)) yaw = (float)lua_tonumber(E->L, -1);
            lua_pop(E->L, 1);
            i++;
        }

        std::cout << "PosX:" << posX << ", PosY:" << posY << ", PosZ:" << posZ << std::endl;
        go->Relocate(posX, posY, posZ, yaw);
        go->SetLocalRotationAngles(yaw, roll, pitch);
        go->DestroyForNearbyPlayers();
        go->UpdateObjectVisibility();

        return 0;
    }

    int SetPosition(Eluna* E, GameObject* go)
    {
        uint8 i = 2;
        float posX = go->GetPositionX();
        float posY = go->GetPositionY();
        float posZ = go->GetPositionZ();
        float pitch = 0.0f;
        float roll = 0.0f;
        float yaw = go->GetOrientation();

        if (lua_istable(E->L, i)) {
            lua_rawgeti(E->L, i, 1);
            if (!lua_isnil(E->L, -1)) posX = (float)lua_tonumber(E->L, -1);
            lua_pop(E->L, 1);

            lua_rawgeti(E->L, i, 2);
            if (!lua_isnil(E->L, -1)) posY = (float)lua_tonumber(E->L, -1);
            lua_pop(E->L, 1);

            lua_rawgeti(E->L, i, 3);
            if (!lua_isnil(E->L, -1)) posZ = (float)lua_tonumber(E->L, -1);
            lua_pop(E->L, 1);
            i++;
        }

        if (lua_istable(E->L, i)) {
            lua_rawgeti(E->L, i, 1);
            if (!lua_isnil(E->L, -1)) pitch = (float)lua_tonumber(E->L, -1);
            lua_pop(E->L, 1);

            lua_rawgeti(E->L, i, 2);
            if (!lua_isnil(E->L, -1)) roll = (float)lua_tonumber(E->L, -1);
            lua_pop(E->L, 1);

            lua_rawgeti(E->L, i, 3);
            if (!lua_isnil(E->L, -1)) yaw = (float)lua_tonumber(E->L, -1);
            lua_pop(E->L, 1);
            i++;
        }

        Map* map = nullptr;
        if (lua_gettop(E->L) >= i && !lua_isnil(E->L, i))
            map = E->CHECKOBJ<Map>(i, false);
        if (!map)
            map = go->GetMap();
        
        ObjectGuid::LowType guidLow = go->GetSpawnId();
        if (!guidLow)
            return luaL_error(E->L, "GameObject must be from DB (spawned).");

        GameObjectData const* data = go->GetGameObjectData();
        if (!data)
            return luaL_error(E->L, "GameObject has no persistent data.");

        Position pos = { posX, posY, posZ };
        
        go->WorldRelocate(map->GetId(), pos);
        go->Relocate(go->GetPositionX(), go->GetPositionY(), go->GetPositionZ(), yaw);
        go->SetLocalRotationAngles(yaw, roll, pitch);

        sObjectMgr->RemoveGameobjectFromGrid(guidLow, go->GetGameObjectData());
        go->SaveToDB();
        sObjectMgr->AddGameobjectToGrid(guidLow, go->GetGameObjectData());

        go->Delete();

        go = new GameObject();
        if (!go->LoadFromDB(guidLow, map, true))
        {
            delete go;
            return false;
        }

        E->Push(go);

        return 1;
    }

    int SetScale(Eluna* E, GameObject* obj)
    {
        float scale = E->CHECKVAL<float>(2);

        if (scale <= 0.0f)
        {
            scale = obj->GetGOInfo()->size;
            const_cast<GameObjectData*>(obj->GetGameObjectData())->size = -1.0f;
        }
        else
        {
            const_cast<GameObjectData*>(obj->GetGameObjectData())->size = scale;
        }

        obj->SetObjectScale(scale);
        obj->DestroyForNearbyPlayers();
        obj->UpdateObjectVisibility();
        obj->SaveToDB();
        
        return 0;
    }

    ElunaRegister<GameObject> GameObjectMethods[] =
    {
        // Getters
        { "GetDisplayId", &LuaGameObject::GetDisplayId },
        { "GetGoState", &LuaGameObject::GetGoState },
        { "GetLootState", &LuaGameObject::GetLootState },
        { "GetLootRecipient", &LuaGameObject::GetLootRecipient },
        { "GetLootRecipientGroup", &LuaGameObject::GetLootRecipientGroup },
        { "GetDBTableGUIDLow", &LuaGameObject::GetDBTableGUIDLow },

        // Setters
        { "SetGoState", &LuaGameObject::SetGoState },
        { "SetLootState", &LuaGameObject::SetLootState },
        { "SetRespawnTime", &LuaGameObject::SetRespawnTime },

        // Boolean
        { "IsTransport", &LuaGameObject::IsTransport },
        { "IsDestructible", &LuaGameObject::IsDestructible },
        { "IsActive", &LuaGameObject::IsActive },
        { "HasQuest", &LuaGameObject::HasQuest },
        { "IsSpawned", &LuaGameObject::IsSpawned },

        // Other
        { "RemoveFromWorld", &LuaGameObject::RemoveFromWorld },
        { "UseDoorOrButton", &LuaGameObject::UseDoorOrButton },
        { "Despawn", &LuaGameObject::Despawn },
        { "Respawn", &LuaGameObject::Respawn },
        { "SaveToDB", &LuaGameObject::SaveToDB },

        { "SetTemporaryPosition", &LuaGameObject::SetTemporaryPosition },
        { "SetPosition", &LuaGameObject::SetPosition },
        { "SetScale", &LuaGameObject::SetScale }
    };
};
#endif
