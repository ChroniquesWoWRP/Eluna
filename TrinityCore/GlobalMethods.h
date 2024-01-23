/*
* Copyright (C) 2010 - 2024 Eluna Lua Engine <https://elunaluaengine.github.io/>
* This program is free software licensed under GPL version 3
* Please see the included DOCS/LICENSE.md for more information
*/

#ifndef GLOBALMETHODS_H
#define GLOBALMETHODS_H

#include "BindingMap.h"

/***
 * These functions can be used anywhere at any time, including at start-up.
 */
namespace LuaGlobalFunctions
{
    /**
     * Returns Lua engine's name.
     *
     * Always returns "ElunaEngine" on Eluna.
     *
     * @return string engineName
     */
    int GetLuaEngine(Eluna* E)
    {
        E->Push("ElunaEngine");
        return 1;
    }

    /**
     * Returns emulator's name.
     *
     * The result will be either `MaNGOS`, `cMaNGOS`, or `TrinityCore`.
     *
     * @return string coreName
     */
    int GetCoreName(Eluna* E)
    {
        E->Push(CORE_NAME);
        return 1;
    }

    /**
     * Returns emulator .conf RealmID
     *
     * - for MaNGOS returns the realmID as it is stored in the core.
     * - for TrinityCore returns the realmID as it is in the conf file.
     * @return uint32 realm ID
     */

    int GetRealmID(Eluna* E)
    {
        E->Push(sConfigMgr->GetIntDefault("RealmID", 1));
        return 1;
    }

    /**
     * Returns emulator version
     *
     * - For TrinityCore returns the date of the last revision, e.g. `2015-08-26 22:53:12 +0300`
     * - For cMaNGOS returns the date and time of the last revision, e.g. `2015-09-06 13:18:50`
     * - for MaNGOS returns the version number as string, e.g. `21000`
     *
     * @return string version
     */
    int GetCoreVersion(Eluna* E)
    {
        E->Push(CORE_VERSION);
        return 1;
    }

    /**
     * Returns emulator's supported expansion.
     *
     * Expansion is 0 for pre-TBC, 1 for TBC, 2 for WotLK, and 3 for Cataclysm.
     *
     * @return int32 expansion
     */
    int GetCoreExpansion(Eluna* E)
    {
#ifdef WOTLK
        E->Push(2);
#elif CATA
        E->Push(3);
#endif
        return 1;
    }

    /**
     * Returns the [Map] pointer of the Lua state. Returns null for the "World" state.
     *
     * @return [Map] map
     */
    int GetStateMap(Eluna* E)
    {
        E->Push(E->GetBoundMap());
        return 1;
    }

    /**
     * Returns the map ID of the Lua state. Returns -1 for the "World" state.
     *
     * @return int32 mapId
     */
    int GetStateMapId(Eluna* E)
    {
        E->Push(E->GetBoundMapId());
        return 1;
    }

    /**
     * Returns the instance ID of the Lua state. Returns 0 for continent maps and the world state.
     *
     * @return uint32 instanceId
     */
    int GetStateInstanceId(Eluna* E)
    {
        E->Push(E->GetBoundInstanceId());
        return 1;
    }

    /**
     * Returns [Quest] template
     *
     * @param uint32 questId : [Quest] entry ID
     * @return [Quest] quest
     */
    int GetQuest(Eluna* E)
    {
        uint32 questId = Eluna::CHECKVAL<uint32>(E->L, 1);

        E->Push(eObjectMgr->GetQuestTemplate(questId));
        return 1;
    }

    /**
     * Finds and Returns [Player] by guid if found
     *
     * In multistate, this method is only available in the WORLD state
     * 
     * @param ObjectGuid guid : guid of the [Player], you can get it with [Object:GetGUID]
     * @return [Player] player
     */
    int GetPlayerByGUID(Eluna* E)
    {
        ObjectGuid guid = Eluna::CHECKVAL<ObjectGuid>(E->L, 1);
        E->Push(eObjectAccessor()FindPlayer(guid));
        return 1;
    }

    /**
     * Finds and Returns [Player] by name if found
     *
     * In multistate, this method is only available in the WORLD state
     *
     * @param string name : name of the [Player]
     * @return [Player] player
     */
    int GetPlayerByName(Eluna* E)
    {
        const char* name = Eluna::CHECKVAL<const char*>(E->L, 1);
        E->Push(eObjectAccessor()FindPlayerByName(name));
        return 1;
    }

    /**
     * Returns game time in seconds
     *
     * @return uint32 time
     */
    int GetGameTime(Eluna* E)
    {
        E->Push(GameTime::GetGameTime());
        return 1;
    }

    /**
     * Returns a table with all the current [Player]s in the world
     *
     * Does not return players that may be teleporting or otherwise not on any map.
     *
     *     enum TeamId
     *     {
     *         TEAM_ALLIANCE = 0,
     *         TEAM_HORDE = 1,
     *         TEAM_NEUTRAL = 2
     *     };
     *
     * In multistate, this method is only available in the WORLD state
     *
     * @param [TeamId] team = TEAM_NEUTRAL : optional check team of the [Player], Alliance, Horde or Neutral (All)
     * @param bool onlyGM = false : optional check if GM only
     * @return table worldPlayers
     */
    int GetPlayersInWorld(Eluna* E)
    {
        uint32 team = Eluna::CHECKVAL<uint32>(E->L, 1, TEAM_NEUTRAL);
        bool onlyGM = Eluna::CHECKVAL<bool>(E->L, 2, false);

        lua_newtable(E->L);
        int tbl = lua_gettop(E->L);
        uint32 i = 0;

        std::shared_lock<std::shared_mutex> lock(*HashMapHolder<Player>::GetLock());
        const HashMapHolder<Player>::MapType& m = eObjectAccessor()GetPlayers();
        for (HashMapHolder<Player>::MapType::const_iterator it = m.begin(); it != m.end(); ++it)
        {
            if (Player* player = it->second)
            {
                if (!player->IsInWorld())
                    continue;

                if ((team == TEAM_NEUTRAL || player->GetTeamId() == team) && (!onlyGM || player->IsGameMaster()))
                {
                    E->Push(player);
                    lua_rawseti(E->L, tbl, ++i);
                }
            }
        }

        lua_settop(E->L, tbl); // push table to top of stack
        return 1;
    }

    /**
     * Returns a [Guild] by name.
     *
     * @param string name
     * @return [Guild] guild : the Guild, or `nil` if it doesn't exist
     */
    int GetGuildByName(Eluna* E)
    {
        const char* name = Eluna::CHECKVAL<const char*>(E->L, 1);
        E->Push(eGuildMgr->GetGuildByName(name));
        return 1;
    }

    /**
     * Returns a [Map] by ID.
     *
     * @param uint32 mapId : see [Map.dbc](https://github.com/cmangos/issues/wiki/Map.dbc)
     * @param uint32 instanceId = 0 : required if the map is an instance, otherwise don't pass anything
     * @return [Map] map : the Map, or `nil` if it doesn't exist
     */
    int GetMapById(Eluna* E)
    {
        uint32 mapid = Eluna::CHECKVAL<uint32>(E->L, 1);
        uint32 instance = Eluna::CHECKVAL<uint32>(E->L, 2, 0);

        E->Push(eMapMgr->FindMap(mapid, instance));
        return 1;
    }

    /**
     * Returns [Guild] by the leader's GUID
     *
     * @param ObjectGuid guid : the guid of a [Guild] leader
     * @return [Guild] guild, or `nil` if it doesn't exist
     */
    int GetGuildByLeaderGUID(Eluna* E)
    {
        ObjectGuid guid = Eluna::CHECKVAL<ObjectGuid>(E->L, 1);

        E->Push(eGuildMgr->GetGuildByLeader(guid));
        return 1;
    }

    /**
     * Returns the amount of [Player]s in the world.
     *
     * @return uint32 count
     */
    int GetPlayerCount(Eluna* E)
    {
        E->Push(eWorld->GetActiveSessionCount());
        return 1;
    }

    /**
     * Builds a [Player]'s GUID
     *
     * [Player] GUID consist of low GUID and type ID
     *
     * [Player] and [Creature] for example can have the same low GUID but not GUID.
     *
     * @param uint32 lowguid : low GUID of the [Player]
     * @return ObjectGuid guid
     */
    int GetPlayerGUID(Eluna* E)
    {
        uint32 lowguid = Eluna::CHECKVAL<uint32>(E->L, 1);
        E->Push(MAKE_NEW_GUID(lowguid, 0, HIGHGUID_PLAYER));
        return 1;
    }

    /**
     * Builds an [Item]'s GUID.
     *
     * [Item] GUID consist of low GUID and type ID
     * [Player] and [Item] for example can have the same low GUID but not GUID.
     *
     * @param uint32 lowguid : low GUID of the [Item]
     * @return ObjectGuid guid
     */
    int GetItemGUID(Eluna* E)
    {
        uint32 lowguid = Eluna::CHECKVAL<uint32>(E->L, 1);
        E->Push(MAKE_NEW_GUID(lowguid, 0, HIGHGUID_ITEM));
        return 1;
    }

    /**
     * Builds a [GameObject]'s GUID.
     *
     * A GameObject's GUID consist of entry ID, low GUID and type ID
     *
     * A [Player] and GameObject for example can have the same low GUID but not GUID.
     *
     * @param uint32 lowguid : low GUID of the [GameObject]
     * @param uint32 entry : entry ID of the [GameObject]
     * @return ObjectGuid guid
     */
    int GetObjectGUID(Eluna* E)
    {
        uint32 lowguid = Eluna::CHECKVAL<uint32>(E->L, 1);
        uint32 entry = Eluna::CHECKVAL<uint32>(E->L, 2);
        E->Push(MAKE_NEW_GUID(lowguid, entry, HIGHGUID_GAMEOBJECT));
        return 1;
    }

    /**
     * Builds a [Creature]'s GUID.
     *
     * [Creature] GUID consist of entry ID, low GUID and type ID
     *
     * [Player] and [Creature] for example can have the same low GUID but not GUID.
     *
     * @param uint32 lowguid : low GUID of the [Creature]
     * @param uint32 entry : entry ID of the [Creature]
     * @return ObjectGuid guid
     */
    int GetUnitGUID(Eluna* E)
    {
        uint32 lowguid = Eluna::CHECKVAL<uint32>(E->L, 1);
        uint32 entry = Eluna::CHECKVAL<uint32>(E->L, 2);
        E->Push(MAKE_NEW_GUID(lowguid, entry, HIGHGUID_UNIT));
        return 1;
    }

    /**
     * Returns the low GUID from a GUID.
     *
     * A GUID consists of a low GUID, type ID, and possibly an entry ID depending on the type ID.
     *
     * Low GUID is an ID to distinct the objects of the same type.
     *
     * [Player] and [Creature] for example can have the same low GUID but not GUID.
     * 
     * On TrinityCore all low GUIDs are different for all objects of the same type.
     * For example creatures in instances are assigned new GUIDs when the Map is created.
     * 
     * On MaNGOS and cMaNGOS low GUIDs are unique only on the same map.
     * For example creatures in instances use the same low GUID assigned for that spawn in the database.
     * This is why to identify a creature you have to know the instanceId and low GUID. See [Map:GetIntstanceId]
     *
     * @param ObjectGuid guid : GUID of an [Object]
     * @return uint32 lowguid : low GUID of the [Object]
     */
    int GetGUIDLow(Eluna* E)
    {
        ObjectGuid guid = Eluna::CHECKVAL<ObjectGuid>(E->L, 1);

        E->Push(guid.GetCounter());
        return 1;
    }

    /**
     * Returns an chat link for an [Item].
     *
     *     enum LocaleConstant
     *     {
     *         LOCALE_enUS = 0,
     *         LOCALE_koKR = 1,
     *         LOCALE_frFR = 2,
     *         LOCALE_deDE = 3,
     *         LOCALE_zhCN = 4,
     *         LOCALE_zhTW = 5,
     *         LOCALE_esES = 6,
     *         LOCALE_esMX = 7,
     *         LOCALE_ruRU = 8
     *     };
     *
     * @param uint32 entry : entry ID of an [Item]
     * @param [LocaleConstant] locale = DEFAULT_LOCALE : locale to return the [Item] name in
     * @return string itemLink
     */
    int GetItemLink(Eluna* E)
    {
        uint32 entry = Eluna::CHECKVAL<uint32>(E->L, 1);
        uint8 locale = Eluna::CHECKVAL<uint8>(E->L, 2, DEFAULT_LOCALE);
        if (locale >= TOTAL_LOCALES)
            return luaL_argerror(E->L, 2, "valid LocaleConstant expected");

        const ItemTemplate* temp = eObjectMgr->GetItemTemplate(entry);
        if (!temp)
            return luaL_argerror(E->L, 1, "valid ItemEntry expected");

#ifdef CATA
        std::string name = temp->ExtendedData->Display->Str[locale];
#else
        std::string name = temp->Name1;
#endif
        if (ItemLocale const* il = eObjectMgr->GetItemLocale(entry))
            ObjectMgr::GetLocaleString(il->Name, static_cast<LocaleConstant>(locale), name);

        std::ostringstream oss;
#ifdef CATA
        oss << "|c" << std::hex << ItemQualityColors[temp->ExtendedData->Quality] << std::dec <<
#else
        oss << "|c" << std::hex << ItemQualityColors[temp->Quality] << std::dec <<
#endif
            "|Hitem:" << entry << ":0:" <<
            "0:0:0:0:" <<
            "0:0:0:0|h[" << name << "]|h|r";

        E->Push(oss.str());
        return 1;
    }

    /**
     * Returns the type ID from a GUID.
     *
     * Type ID is different for each type ([Player], [Creature], [GameObject], etc.).
     *
     * GUID consist of entry ID, low GUID, and type ID.
     *
     * @param ObjectGuid guid : GUID of an [Object]
     * @return int32 typeId : type ID of the [Object]
     */
    int GetGUIDType(Eluna* E)
    {
        ObjectGuid guid = Eluna::CHECKVAL<ObjectGuid>(E->L, 1);
        E->Push(static_cast<int>(guid.GetHigh()));
        return 1;
    }

    /**
     * Returns the entry ID from a GUID.
     *
     * GUID consist of entry ID, low GUID, and type ID.
     *
     * @param ObjectGuid guid : GUID of an [Creature] or [GameObject]
     * @return uint32 entry : entry ID, or `0` if `guid` is not a [Creature] or [GameObject]
     */
    int GetGUIDEntry(Eluna* E)
    {
        ObjectGuid guid = Eluna::CHECKVAL<ObjectGuid>(E->L, 1);
        E->Push(guid.GetEntry());
        return 1;
    }

    /**
     * Returns the area or zone's name.
     *
     *     enum LocaleConstant
     *     {
     *         LOCALE_enUS = 0,
     *         LOCALE_koKR = 1,
     *         LOCALE_frFR = 2,
     *         LOCALE_deDE = 3,
     *         LOCALE_zhCN = 4,
     *         LOCALE_zhTW = 5,
     *         LOCALE_esES = 6,
     *         LOCALE_esMX = 7,
     *         LOCALE_ruRU = 8
     *     };
     *
     * @param uint32 areaOrZoneId : area ID or zone ID
     * @param [LocaleConstant] locale = DEFAULT_LOCALE : locale to return the name in
     * @return string areaOrZoneName
     */
    int GetAreaName(Eluna* E)
    {
        uint32 areaOrZoneId = Eluna::CHECKVAL<uint32>(E->L, 1);
        uint8 locale = Eluna::CHECKVAL<uint8>(E->L, 2, DEFAULT_LOCALE);
        if (locale >= TOTAL_LOCALES)
            return luaL_argerror(E->L, 2, "valid LocaleConstant expected");

        AreaTableEntry const* areaEntry = sAreaTableStore.LookupEntry(areaOrZoneId);
        if (!areaEntry)
            return luaL_argerror(E->L, 1, "valid Area or Zone ID expected");

        E->Push(areaEntry->AreaName[locale]);
        return 1;
    }

    /**
     * Returns the currently active game events.
     *
     * @return table activeEvents
     */
    int GetActiveGameEvents(Eluna* E)
    {
        lua_newtable(E->L);
        int tbl = lua_gettop(E->L);
        uint32 counter = 1;
        GameEventMgr::ActiveEvents const& activeEvents = eGameEventMgr->GetActiveEventList();

        for (GameEventMgr::ActiveEvents::const_iterator i = activeEvents.begin(); i != activeEvents.end(); ++i)
        {
            E->Push(*i);
            lua_rawseti(E->L, tbl, counter);

            counter++;
        }

        lua_settop(E->L, tbl);
        return 1;
    }

    static int RegisterEntryHelper(Eluna* E, int regtype)
    {
        uint32 id = Eluna::CHECKVAL<uint32>(E->L, 1);
        uint32 ev = Eluna::CHECKVAL<uint32>(E->L, 2);
        luaL_checktype(E->L, 3, LUA_TFUNCTION);
        uint32 shots = Eluna::CHECKVAL<uint32>(E->L, 4, 0);

        lua_pushvalue(E->L, 3);
        int functionRef = luaL_ref(E->L, LUA_REGISTRYINDEX);
        if (functionRef >= 0)
            return E->Register(E->L, regtype, id, ObjectGuid(), 0, ev, functionRef, shots);
        else
            luaL_argerror(E->L, 3, "unable to make a ref to function");
        return 0;
    }

    static int RegisterEventHelper(Eluna* E, int regtype)
    {
        uint32 ev = Eluna::CHECKVAL<uint32>(E->L, 1);
        luaL_checktype(E->L, 2, LUA_TFUNCTION);
        uint32 shots = Eluna::CHECKVAL<uint32>(E->L, 3, 0);

        lua_pushvalue(E->L, 2);
        int functionRef = luaL_ref(E->L, LUA_REGISTRYINDEX);
        if (functionRef >= 0)
            return E->Register(E->L, regtype, 0, ObjectGuid(), 0, ev, functionRef, shots);
        else
            luaL_argerror(E->L, 2, "unable to make a ref to function");
        return 0;
    }

    static int RegisterUniqueHelper(Eluna* E, int regtype)
    {
        ObjectGuid guid = Eluna::CHECKVAL<ObjectGuid>(E->L, 1);
        uint32 instanceId = Eluna::CHECKVAL<uint32>(E->L, 2);
        uint32 ev = Eluna::CHECKVAL<uint32>(E->L, 3);
        luaL_checktype(E->L, 4, LUA_TFUNCTION);
        uint32 shots = Eluna::CHECKVAL<uint32>(E->L, 5, 0);

        lua_pushvalue(E->L, 4);
        int functionRef = luaL_ref(E->L, LUA_REGISTRYINDEX);
        if (functionRef >= 0)
            return E->Register(E->L, regtype, 0, guid, instanceId, ev, functionRef, shots);
        else
            luaL_argerror(E->L, 4, "unable to make a ref to function");
        return 0;
    }

    /**
     * Registers a server event handler.
     *
     *     enum ServerEvents
     *     {
     *         // Server
     *         SERVER_EVENT_ON_NETWORK_START           =     1,       // Not Implemented
     *         SERVER_EVENT_ON_NETWORK_STOP            =     2,       // Not Implemented
     *         SERVER_EVENT_ON_SOCKET_OPEN             =     3,       // Not Implemented
     *         SERVER_EVENT_ON_SOCKET_CLOSE            =     4,       // Not Implemented
     *         SERVER_EVENT_ON_PACKET_RECEIVE          =     5,       // (event, packet, player) - Player only if accessible. Can return false, newPacket
     *         SERVER_EVENT_ON_PACKET_RECEIVE_UNKNOWN  =     6,       // Not Implemented
     *         SERVER_EVENT_ON_PACKET_SEND             =     7,       // (event, packet, player) - Player only if accessible. Can return false
     *
     *         // World
     *         WORLD_EVENT_ON_OPEN_STATE_CHANGE        =     8,        // (event, open) - Needs core support on Mangos
     *         WORLD_EVENT_ON_CONFIG_LOAD              =     9,        // (event, reload)
     *         // UNUSED                               =     10,
     *         WORLD_EVENT_ON_SHUTDOWN_INIT            =     11,       // (event, code, mask)
     *         WORLD_EVENT_ON_SHUTDOWN_CANCEL          =     12,       // (event)
     *         WORLD_EVENT_ON_UPDATE                   =     13,       // (event, diff)
     *         WORLD_EVENT_ON_STARTUP                  =     14,       // (event)
     *         WORLD_EVENT_ON_SHUTDOWN                 =     15,       // (event)
     *
     *         // Eluna
     *         ELUNA_EVENT_ON_LUA_STATE_CLOSE          =     16,       // (event) - triggers just before shutting down eluna (on shutdown and restart)
     *
     *         // Map
     *         MAP_EVENT_ON_CREATE                     =     17,       // (event, map)
     *         MAP_EVENT_ON_DESTROY                    =     18,       // (event, map)
     *         MAP_EVENT_ON_GRID_LOAD                  =     19,       // Not Implemented
     *         MAP_EVENT_ON_GRID_UNLOAD                =     20,       // Not Implemented
     *         MAP_EVENT_ON_PLAYER_ENTER               =     21,       // (event, map, player)
     *         MAP_EVENT_ON_PLAYER_LEAVE               =     22,       // (event, map, player)
     *         MAP_EVENT_ON_UPDATE                     =     23,       // (event, map, diff)
     *
     *         // Area trigger
     *         TRIGGER_EVENT_ON_TRIGGER                =     24,       // (event, player, triggerId) - Can return true
     *
     *         // Weather
     *         WEATHER_EVENT_ON_CHANGE                 =     25,       // (event, zoneId, state, grade)
     *
     *         // Auction house
     *         AUCTION_EVENT_ON_ADD                    =     26,       // (event, auctionId, owner, item, expireTime, buyout, startBid, currentBid, bidderGUIDLow)
     *         AUCTION_EVENT_ON_REMOVE                 =     27,       // (event, auctionId, owner, item, expireTime, buyout, startBid, currentBid, bidderGUIDLow)
     *         AUCTION_EVENT_ON_SUCCESSFUL             =     28,       // (event, auctionId, owner, item, expireTime, buyout, startBid, currentBid, bidderGUIDLow)
     *         AUCTION_EVENT_ON_EXPIRE                 =     29,       // (event, auctionId, owner, item, expireTime, buyout, startBid, currentBid, bidderGUIDLow)
     *
     *         // AddOns
     *         ADDON_EVENT_ON_MESSAGE                  =     30,       // (event, sender, type, prefix, msg, target) - target can be nil/whisper_target/guild/group/channel. Can return false
     *
     *         WORLD_EVENT_ON_DELETE_CREATURE          =     31,       // (event, creature)
     *         WORLD_EVENT_ON_DELETE_GAMEOBJECT        =     32,       // (event, gameobject)
     *
     *         // Eluna
     *         ELUNA_EVENT_ON_LUA_STATE_OPEN           =     33,       // (event) - triggers after all scripts are loaded
     *
     *         GAME_EVENT_START                        =     34,       // (event, gameeventid)
     *         GAME_EVENT_STOP                         =     35,       // (event, gameeventid)
     *     };
     *
     * @proto cancel = (event, function)
     * @proto cancel = (event, function, shots)
     *
     * @param uint32 event : server event ID, refer to ServerEvents above
     * @param function function : function that will be called when the event occurs
     * @param uint32 shots = 0 : the number of times the function will be called, 0 means "always call this function"
     *
     * @return function cancel : a function that cancels the binding when called
     */
    int RegisterServerEvent(Eluna* E)
    {
        return RegisterEventHelper(E, Hooks::REGTYPE_SERVER);
    }

    /**
     * Registers a [Player] event handler.
     *
     * <pre>
     * enum PlayerEvents
     * {
     *     PLAYER_EVENT_ON_CHARACTER_CREATE        =     1,        // (event, player)
     *     PLAYER_EVENT_ON_CHARACTER_DELETE        =     2,        // (event, guid)
     *     PLAYER_EVENT_ON_LOGIN                   =     3,        // (event, player)
     *     PLAYER_EVENT_ON_LOGOUT                  =     4,        // (event, player)
     *     PLAYER_EVENT_ON_SPELL_CAST              =     5,        // (event, player, spell, skipCheck)
     *     PLAYER_EVENT_ON_KILL_PLAYER             =     6,        // (event, killer, killed)
     *     PLAYER_EVENT_ON_KILL_CREATURE           =     7,        // (event, killer, killed)
     *     PLAYER_EVENT_ON_KILLED_BY_CREATURE      =     8,        // (event, killer, killed)
     *     PLAYER_EVENT_ON_DUEL_REQUEST            =     9,        // (event, target, challenger)
     *     PLAYER_EVENT_ON_DUEL_START              =     10,       // (event, player1, player2)
     *     PLAYER_EVENT_ON_DUEL_END                =     11,       // (event, winner, loser, type)
     *     PLAYER_EVENT_ON_GIVE_XP                 =     12,       // (event, player, amount, victim) - Can return new XP amount
     *     PLAYER_EVENT_ON_LEVEL_CHANGE            =     13,       // (event, player, oldLevel)
     *     PLAYER_EVENT_ON_MONEY_CHANGE            =     14,       // (event, player, amount) - Can return new money amount
     *     PLAYER_EVENT_ON_REPUTATION_CHANGE       =     15,       // (event, player, factionId, standing, incremental) - Can return new standing
     *     PLAYER_EVENT_ON_TALENTS_CHANGE          =     16,       // (event, player, points)
     *     PLAYER_EVENT_ON_TALENTS_RESET           =     17,       // (event, player, noCost)
     *     PLAYER_EVENT_ON_CHAT                    =     18,       // (event, player, msg, Type, lang) - Can return false, newMessage
     *     PLAYER_EVENT_ON_WHISPER                 =     19,       // (event, player, msg, Type, lang, receiver) - Can return false, newMessage
     *     PLAYER_EVENT_ON_GROUP_CHAT              =     20,       // (event, player, msg, Type, lang, group) - Can return false, newMessage
     *     PLAYER_EVENT_ON_GUILD_CHAT              =     21,       // (event, player, msg, Type, lang, guild) - Can return false, newMessage
     *     PLAYER_EVENT_ON_CHANNEL_CHAT            =     22,       // (event, player, msg, Type, lang, channel) - Can return false, newMessage
     *     PLAYER_EVENT_ON_EMOTE                   =     23,       // (event, player, emote) - Not triggered on any known emote
     *     PLAYER_EVENT_ON_TEXT_EMOTE              =     24,       // (event, player, textEmote, emoteNum, guid)
     *     PLAYER_EVENT_ON_SAVE                    =     25,       // (event, player)
     *     PLAYER_EVENT_ON_BIND_TO_INSTANCE        =     26,       // (event, player, difficulty, mapid, permanent)
     *     PLAYER_EVENT_ON_UPDATE_ZONE             =     27,       // (event, player, newZone, newArea)
     *     PLAYER_EVENT_ON_MAP_CHANGE              =     28,       // (event, player)
     *
     *     // Custom
     *     PLAYER_EVENT_ON_EQUIP                   =     29,       // (event, player, item, bag, slot)
     *     PLAYER_EVENT_ON_FIRST_LOGIN             =     30,       // (event, player)
     *     PLAYER_EVENT_ON_CAN_USE_ITEM            =     31,       // (event, player, itemEntry) - Can return InventoryResult enum value
     *     PLAYER_EVENT_ON_LOOT_ITEM               =     32,       // (event, player, item, count)
     *     PLAYER_EVENT_ON_ENTER_COMBAT            =     33,       // (event, player, enemy)
     *     PLAYER_EVENT_ON_LEAVE_COMBAT            =     34,       // (event, player)
     *     PLAYER_EVENT_ON_REPOP                   =     35,       // (event, player)
     *     PLAYER_EVENT_ON_RESURRECT               =     36,       // (event, player)
     *     PLAYER_EVENT_ON_LOOT_MONEY              =     37,       // (event, player, amount)
     *     PLAYER_EVENT_ON_QUEST_ABANDON           =     38,       // (event, player, questId)
     *     PLAYER_EVENT_ON_LEARN_TALENTS           =     39,       // (event, player, talentId, talentRank, spellid)
     *     PLAYER_EVENT_ON_ENVIRONMENTAL_DEATH     =     40,       // (event, player, environmentalDamageType)
     *     PLAYER_EVENT_ON_TRADE_ACCEPT            =     41,       // (event, player, target) - Can return false to interrupt trade
     *     PLAYER_EVENT_ON_COMMAND                 =     42,       // (event, player, command) - player is nil if command used from console. Can return false
     *     PLAYER_EVENT_ON_SKILL_CHANGE            =     43,       // (event, player, skillId, skillValue) - Returns new skill level value
     *     PLAYER_EVENT_ON_LEARN_SPELL             =     44,       // (event, player, spellId)
     *     PLAYER_EVENT_ON_ACHIEVEMENT_COMPLETE    =     45,       // (event, player, achievementId)
     *     // UNUSED                               =     46,       // (event, player)
     *     PLAYER_EVENT_ON_UPDATE_AREA             =     47,       // (event, player, oldArea, newArea)
     *     PLAYER_EVENT_ON_TRADE_INIT              =     48,       // (event, player, target) - Can return false to interrupt trade
     *     PLAYER_EVENT_ON_SEND_MAIL               =     49,       // (event, player, recipientGuid) - Can return false to interrupt sending
     *     // UNUSED                               =     50,       // (event, player)
     *     // UNUSED                               =     51,       // (event, player)
     *     // UNUSED                               =     52,       // (event, player)
     *     // UNUSED                               =     53,       // (event, player)
     *     PLAYER_EVENT_ON_QUEST_STATUS_CHANGED    =     54,       // (event, player, questId, status)
     * };
     * </pre>
     *
     * @proto cancel = (event, function)
     * @proto cancel = (event, function, shots)
     *
     * @param uint32 event : [Player] event Id, refer to PlayerEvents above
     * @param function function : function to register
     * @param uint32 shots = 0 : the number of times the function will be called, 0 means "always call this function"
     *
     * @return function cancel : a function that cancels the binding when called
     */
    int RegisterPlayerEvent(Eluna* E)
    {
        return RegisterEventHelper(E, Hooks::REGTYPE_PLAYER);
    }

    /**
     * Registers a [Guild] event handler.
     *
     * <pre>
     * enum GuildEvents
     * {
     *     // Guild
     *     GUILD_EVENT_ON_ADD_MEMBER               =     1,       // (event, guild, player, rank)
     *     GUILD_EVENT_ON_REMOVE_MEMBER            =     2,       // (event, guild, player, isDisbanding)
     *     GUILD_EVENT_ON_MOTD_CHANGE              =     3,       // (event, guild, newMotd)
     *     GUILD_EVENT_ON_INFO_CHANGE              =     4,       // (event, guild, newInfo)
     *     GUILD_EVENT_ON_CREATE                   =     5,       // (event, guild, leader, name)  // Not on TC
     *     GUILD_EVENT_ON_DISBAND                  =     6,       // (event, guild)
     *     GUILD_EVENT_ON_MONEY_WITHDRAW           =     7,       // (event, guild, player, amount, isRepair) - Can return new money amount
     *     GUILD_EVENT_ON_MONEY_DEPOSIT            =     8,       // (event, guild, player, amount) - Can return new money amount
     *     GUILD_EVENT_ON_ITEM_MOVE                =     9,       // (event, guild, player, item, isSrcBank, srcContainer, srcSlotId, isDestBank, destContainer, destSlotId)   // TODO
     *     GUILD_EVENT_ON_EVENT                    =     10,      // (event, guild, eventType, plrGUIDLow1, plrGUIDLow2, newRank)  // TODO
     *     GUILD_EVENT_ON_BANK_EVENT               =     11,      // (event, guild, eventType, tabId, playerGUIDLow, itemOrMoney, itemStackCount, destTabId)
     *
     *     GUILD_EVENT_COUNT
     * };
     * </pre>
     *
     * @proto cancel = (event, function)
     * @proto cancel = (event, function, shots)
     *
     * @param uint32 event : [Guild] event Id, refer to GuildEvents above
     * @param function function : function to register
     * @param uint32 shots = 0 : the number of times the function will be called, 0 means "always call this function"
     *
     * @return function cancel : a function that cancels the binding when called
     */
    int RegisterGuildEvent(Eluna* E)
    {
        return RegisterEventHelper(E, Hooks::REGTYPE_GUILD);
    }

    /**
     * Registers a [Group] event handler.
     *
     * <pre>
     * enum GroupEvents
     * {
     *     // Group
     *     GROUP_EVENT_ON_MEMBER_ADD               =     1,       // (event, group, guid)
     *     GROUP_EVENT_ON_MEMBER_INVITE            =     2,       // (event, group, guid)
     *     GROUP_EVENT_ON_MEMBER_REMOVE            =     3,       // (event, group, guid, method, kicker, reason)
     *     GROUP_EVENT_ON_LEADER_CHANGE            =     4,       // (event, group, newLeaderGuid, oldLeaderGuid)
     *     GROUP_EVENT_ON_DISBAND                  =     5,       // (event, group)
     *     GROUP_EVENT_ON_CREATE                   =     6,       // (event, group, leaderGuid, groupType)
     *     GROUP_EVENT_ON_MEMBER_ACCEPT            =     7,       // (event, group, player) - Can return false to disable accepting
     *
     *     GROUP_EVENT_COUNT
     * };
     * </pre>
     *
     * @proto cancel = (event, function)
     * @proto cancel = (event, function, shots)
     *
     * @param uint32 event : [Group] event Id, refer to GroupEvents above
     * @param function function : function to register
     * @param uint32 shots = 0 : the number of times the function will be called, 0 means "always call this function"
     *
     * @return function cancel : a function that cancels the binding when called
     */
    int RegisterGroupEvent(Eluna* E)
    {
        return RegisterEventHelper(E, Hooks::REGTYPE_GROUP);
    }

    /**
     * Registers a [BattleGround] event handler.
     *
     * <pre>
     * enum BGEvents
     * {
     *     BG_EVENT_ON_START                               = 1,    // (event, bg, bgId, instanceId) - Needs to be added to TC
     *     BG_EVENT_ON_END                                 = 2,    // (event, bg, bgId, instanceId, winner) - Needs to be added to TC
     *     BG_EVENT_ON_CREATE                              = 3,    // (event, bg, bgId, instanceId) - Needs to be added to TC
     *     BG_EVENT_ON_PRE_DESTROY                         = 4,    // (event, bg, bgId, instanceId) - Needs to be added to TC
     *     BG_EVENT_COUNT
     * };
     * </pre>
     *
     * @proto cancel = (event, function)
     * @proto cancel = (event, function, shots)
     *
     * @param uint32 event : [BattleGround] event Id, refer to BGEvents above
     * @param function function : function to register
     * @param uint32 shots = 0 : the number of times the function will be called, 0 means "always call this function"
     *
     * @return function cancel : a function that cancels the binding when called
     */
    int RegisterBGEvent(Eluna* E)
    {
        return RegisterEventHelper(E, Hooks::REGTYPE_BG);
    }

    /**
     * Registers a [WorldPacket] event handler.
     *
     * <pre>
     * enum PacketEvents
     * {
     *     PACKET_EVENT_ON_PACKET_RECEIVE          =     5,       // (event, packet, player) - Player only if accessible. Can return false, newPacket
     *     PACKET_EVENT_ON_PACKET_RECEIVE_UNKNOWN  =     6,       // Not Implemented
     *     PACKET_EVENT_ON_PACKET_SEND             =     7,       // (event, packet, player) - Player only if accessible. Can return false
     *
     *     PACKET_EVENT_COUNT
     * };
     * </pre>
     *
     * @proto cancel = (entry, event, function)
     * @proto cancel = (entry, event, function, shots)
     *
     * @param uint32 entry : opcode
     * @param uint32 event : packet event Id, refer to PacketEvents above
     * @param function function : function to register
     * @param uint32 shots = 0 : the number of times the function will be called, 0 means "always call this function"
     *
     * @return function cancel : a function that cancels the binding when called
     */
    int RegisterPacketEvent(Eluna* E)
    {
        return RegisterEntryHelper(E, Hooks::REGTYPE_PACKET);
    }

    /**
     * Registers a [Creature] gossip event handler.
     *
     * <pre>
     * enum GossipEvents
     * {
     *     GOSSIP_EVENT_ON_HELLO                           = 1,    // (event, player, object) - Object is the Creature/GameObject/Item. Can return false to do default action. For item gossip can return false to stop spell casting.
     *     GOSSIP_EVENT_ON_SELECT                          = 2,    // (event, player, object, sender, intid, code, menu_id) - Object is the Creature/GameObject/Item/Player, menu_id is only for player gossip. Can return false to do default action.
     *     GOSSIP_EVENT_COUNT
     * };
     * </pre>
     *
     * @proto cancel = (entry, event, function)
     * @proto cancel = (entry, event, function, shots)
     *
     * @param uint32 entry : [Creature] entry Id
     * @param uint32 event : [Creature] gossip event Id, refer to GossipEvents above
     * @param function function : function to register
     * @param uint32 shots = 0 : the number of times the function will be called, 0 means "always call this function"
     *
     * @return function cancel : a function that cancels the binding when called
     */
    int RegisterCreatureGossipEvent(Eluna* E)
    {
        return RegisterEntryHelper(E, Hooks::REGTYPE_CREATURE_GOSSIP);
    }

    /**
     * Registers a [GameObject] gossip event handler.
     *
     * <pre>
     * enum GossipEvents
     * {
     *     GOSSIP_EVENT_ON_HELLO                           = 1,    // (event, player, object) - Object is the Creature/GameObject/Item. Can return false to do default action. For item gossip can return false to stop spell casting.
     *     GOSSIP_EVENT_ON_SELECT                          = 2,    // (event, player, object, sender, intid, code, menu_id) - Object is the Creature/GameObject/Item/Player, menu_id is only for player gossip. Can return false to do default action.
     *     GOSSIP_EVENT_COUNT
     * };
     * </pre>
     *
     * @proto cancel = (entry, event, function)
     * @proto cancel = (entry, event, function, shots)
     *
     * @param uint32 entry : [GameObject] entry Id
     * @param uint32 event : [GameObject] gossip event Id, refer to GossipEvents above
     * @param function function : function to register
     * @param uint32 shots = 0 : the number of times the function will be called, 0 means "always call this function"
     *
     * @return function cancel : a function that cancels the binding when called
     */
    int RegisterGameObjectGossipEvent(Eluna* E)
    {
        return RegisterEntryHelper(E, Hooks::REGTYPE_GAMEOBJECT_GOSSIP);
    }

    /**
     * Registers an [Item] event handler.
     *
     * <pre>
     * enum ItemEvents
     * {
     *     ITEM_EVENT_ON_DUMMY_EFFECT                      = 1,    // (event, caster, spellid, effindex, item)
     *     ITEM_EVENT_ON_USE                               = 2,    // (event, player, item, target) - Can return false to stop the spell casting
     *     ITEM_EVENT_ON_QUEST_ACCEPT                      = 3,    // (event, player, item, quest) - Can return true
     *     ITEM_EVENT_ON_EXPIRE                            = 4,    // (event, player, itemid) - Can return true
     *     ITEM_EVENT_ON_REMOVE                            = 5,    // (event, player, item) - Can return true
     *     ITEM_EVENT_ON_ADD                               = 6,    // (event, player, item)
     *     ITEM_EVENT_COUNT
     * };
     * </pre>
     *
     * @proto cancel = (entry, event, function)
     * @proto cancel = (entry, event, function, shots)
     *
     * @param uint32 entry : [Item] entry Id
     * @param uint32 event : [Item] event Id, refer to ItemEvents above
     * @param function function : function to register
     * @param uint32 shots = 0 : the number of times the function will be called, 0 means "always call this function"
     *
     * @return function cancel : a function that cancels the binding when called
     */
    int RegisterItemEvent(Eluna* E)
    {
        return RegisterEntryHelper(E, Hooks::REGTYPE_ITEM);
    }

    /**
     * Registers an [Item] gossip event handler.
     *
     * <pre>
     * enum GossipEvents
     * {
     *     GOSSIP_EVENT_ON_HELLO                           = 1,    // (event, player, object) - Object is the Creature/GameObject/Item. Can return false to do default action. For item gossip can return false to stop spell casting.
     *     GOSSIP_EVENT_ON_SELECT                          = 2,    // (event, player, object, sender, intid, code, menu_id) - Object is the Creature/GameObject/Item/Player, menu_id is only for player gossip. Can return false to do default action.
     *     GOSSIP_EVENT_COUNT
     * };
     * </pre>
     *
     * @proto cancel = (entry, event, function)
     * @proto cancel = (entry, event, function, shots)
     *
     * @param uint32 entry : [Item] entry Id
     * @param uint32 event : [Item] gossip event Id, refer to GossipEvents above
     * @param function function : function to register
     * @param uint32 shots = 0 : the number of times the function will be called, 0 means "always call this function"
     *
     * @return function cancel : a function that cancels the binding when called
     */
    int RegisterItemGossipEvent(Eluna* E)
    {
        return RegisterEntryHelper(E, Hooks::REGTYPE_ITEM_GOSSIP);
    }

    /**
     * Registers a [Map] event handler for all instance of a [Map].
     *
     * <pre>
     * enum InstanceEvents
     * {
     *     INSTANCE_EVENT_ON_INITIALIZE                    = 1,    // (event, instance_data, map)
     *     INSTANCE_EVENT_ON_LOAD                          = 2,    // (event, instance_data, map)
     *     INSTANCE_EVENT_ON_UPDATE                        = 3,    // (event, instance_data, map, diff)
     *     INSTANCE_EVENT_ON_PLAYER_ENTER                  = 4,    // (event, instance_data, map, player)
     *     INSTANCE_EVENT_ON_CREATURE_CREATE               = 5,    // (event, instance_data, map, creature)
     *     INSTANCE_EVENT_ON_GAMEOBJECT_CREATE             = 6,    // (event, instance_data, map, go)
     *     INSTANCE_EVENT_ON_CHECK_ENCOUNTER_IN_PROGRESS   = 7,    // (event, instance_data, map)
     *     INSTANCE_EVENT_COUNT
     * };
     * </pre>
     *
     * @param uint32 map_id : ID of a [Map]
     * @param uint32 event : [Map] event ID, refer to MapEvents above
     * @param function function : function to register
     * @param uint32 shots = 0 : the number of times the function will be called, 0 means "always call this function"
     */
    int RegisterMapEvent(Eluna* E)
    {
        return RegisterEntryHelper(E, Hooks::REGTYPE_MAP);
    }

    /**
     * Registers a [Map] event handler for one instance of a [Map].
     *
     * <pre>
     * enum InstanceEvents
     * {
     *     INSTANCE_EVENT_ON_INITIALIZE                    = 1,    // (event, instance_data, map)
     *     INSTANCE_EVENT_ON_LOAD                          = 2,    // (event, instance_data, map)
     *     INSTANCE_EVENT_ON_UPDATE                        = 3,    // (event, instance_data, map, diff)
     *     INSTANCE_EVENT_ON_PLAYER_ENTER                  = 4,    // (event, instance_data, map, player)
     *     INSTANCE_EVENT_ON_CREATURE_CREATE               = 5,    // (event, instance_data, map, creature)
     *     INSTANCE_EVENT_ON_GAMEOBJECT_CREATE             = 6,    // (event, instance_data, map, go)
     *     INSTANCE_EVENT_ON_CHECK_ENCOUNTER_IN_PROGRESS   = 7,    // (event, instance_data, map)
     *     INSTANCE_EVENT_COUNT
     * };
     * </pre>
     *
     * @param uint32 instance_id : ID of an instance of a [Map]
     * @param uint32 event : [Map] event ID, refer to MapEvents above
     * @param function function : function to register
     * @param uint32 shots = 0 : the number of times the function will be called, 0 means "always call this function"
     */
    int RegisterInstanceEvent(Eluna* E)
    {
        return RegisterEntryHelper(E, Hooks::REGTYPE_INSTANCE);
    }

    /**
     * Registers a [Player] gossip event handler.
     *
     * Note that you can not use `GOSSIP_EVENT_ON_HELLO` with this hook. It does nothing since players dont have an "on hello".
     *
     * <pre>
     * enum GossipEvents
     * {
     *     GOSSIP_EVENT_ON_HELLO                           = 1,    // (event, player, object) - Object is the Creature/GameObject/Item. Can return false to do default action. For item gossip can return false to stop spell casting.
     *     GOSSIP_EVENT_ON_SELECT                          = 2,    // (event, player, object, sender, intid, code, menu_id) - Object is the Creature/GameObject/Item/Player, menu_id is only for player gossip. Can return false to do default action.
     *     GOSSIP_EVENT_COUNT
     * };
     * </pre>
     *
     * @proto cancel = (menu_id, event, function)
     * @proto cancel = (menu_id, event, function, shots)
     *
     * @param uint32 menu_id : [Player] gossip menu Id
     * @param uint32 event : [Player] gossip event Id, refer to GossipEvents above
     * @param function function : function to register
     * @param uint32 shots = 0 : the number of times the function will be called, 0 means "always call this function"
     *
     * @return function cancel : a function that cancels the binding when called
     */
    int RegisterPlayerGossipEvent(Eluna* E)
    {
        return RegisterEntryHelper(E, Hooks::REGTYPE_PLAYER_GOSSIP);
    }

    /**
     * Registers a [Creature] event handler.
     *
     * <pre>
     * enum CreatureEvents
     * {
     *     CREATURE_EVENT_ON_ENTER_COMBAT                    = 1,  // (event, creature, target) - Can return true to stop normal action
     *     CREATURE_EVENT_ON_LEAVE_COMBAT                    = 2,  // (event, creature) - Can return true to stop normal action
     *     CREATURE_EVENT_ON_TARGET_DIED                     = 3,  // (event, creature, victim) - Can return true to stop normal action
     *     CREATURE_EVENT_ON_DIED                            = 4,  // (event, creature, killer) - Can return true to stop normal action
     *     CREATURE_EVENT_ON_SPAWN                           = 5,  // (event, creature) - Can return true to stop normal action
     *     CREATURE_EVENT_ON_REACH_WP                        = 6,  // (event, creature, type, id) - Can return true to stop normal action
     *     CREATURE_EVENT_ON_AIUPDATE                        = 7,  // (event, creature, diff) - Can return true to stop normal action
     *     CREATURE_EVENT_ON_RECEIVE_EMOTE                   = 8,  // (event, creature, player, emoteid) - Can return true to stop normal action
     *     CREATURE_EVENT_ON_DAMAGE_TAKEN                    = 9,  // (event, creature, attacker, damage) - Can return true to stop normal action, can return new damage as second return value.
     *     CREATURE_EVENT_ON_PRE_COMBAT                      = 10, // (event, creature, target) - Can return true to stop normal action
     *     // UNUSED
     *     CREATURE_EVENT_ON_OWNER_ATTACKED                  = 12, // (event, creature, target) - Can return true to stop normal action            // Not on mangos
     *     CREATURE_EVENT_ON_OWNER_ATTACKED_AT               = 13, // (event, creature, attacker) - Can return true to stop normal action          // Not on mangos
     *     CREATURE_EVENT_ON_HIT_BY_SPELL                    = 14, // (event, creature, caster, spellid) - Can return true to stop normal action
     *     CREATURE_EVENT_ON_SPELL_HIT_TARGET                = 15, // (event, creature, target, spellid) - Can return true to stop normal action
     *     // UNUSED                                         = 16, // (event, creature)
     *     // UNUSED                                         = 17, // (event, creature)
     *     // UNUSED                                         = 18, // (event, creature)
     *     CREATURE_EVENT_ON_JUST_SUMMONED_CREATURE          = 19, // (event, creature, summon) - Can return true to stop normal action
     *     CREATURE_EVENT_ON_SUMMONED_CREATURE_DESPAWN       = 20, // (event, creature, summon) - Can return true to stop normal action
     *     CREATURE_EVENT_ON_SUMMONED_CREATURE_DIED          = 21, // (event, creature, summon, killer) - Can return true to stop normal action    // Not on mangos
     *     CREATURE_EVENT_ON_SUMMONED                        = 22, // (event, creature, summoner) - Can return true to stop normal action
     *     CREATURE_EVENT_ON_RESET                           = 23, // (event, creature)
     *     CREATURE_EVENT_ON_REACH_HOME                      = 24, // (event, creature) - Can return true to stop normal action
     *     // UNUSED                                         = 25, // (event, creature)
     *     CREATURE_EVENT_ON_CORPSE_REMOVED                  = 26, // (event, creature, respawndelay) - Can return true to stop normal action, can return new respawndelay as second return value
     *     CREATURE_EVENT_ON_MOVE_IN_LOS                     = 27, // (event, creature, unit) - Can return true to stop normal action. Does not actually check LOS, just uses the sight range
     *     // UNUSED                                         = 28, // (event, creature)
     *     // UNUSED                                         = 29, // (event, creature)
     *     CREATURE_EVENT_ON_DUMMY_EFFECT                    = 30, // (event, caster, spellid, effindex, creature)
     *     CREATURE_EVENT_ON_QUEST_ACCEPT                    = 31, // (event, player, creature, quest) - Can return true
     *     // UNUSED                                         = 32, // (event, creature)
     *     // UNUSED                                         = 33, // (event, creature)
     *     CREATURE_EVENT_ON_QUEST_REWARD                    = 34, // (event, player, creature, quest, opt) - Can return true
     *     CREATURE_EVENT_ON_DIALOG_STATUS                   = 35, // (event, player, creature)
     *     CREATURE_EVENT_ON_ADD                             = 36, // (event, creature)
     *     CREATURE_EVENT_ON_REMOVE                          = 37, // (event, creature)
     *     CREATURE_EVENT_COUNT
     * };
     * </pre>
     *
     * @proto cancel = (entry, event, function)
     * @proto cancel = (entry, event, function, shots)
     *
     * @param uint32 entry : the ID of one or more [Creature]s
     * @param uint32 event : refer to CreatureEvents above
     * @param function function : function that will be called when the event occurs
     * @param uint32 shots = 0 : the number of times the function will be called, 0 means "always call this function"
     *
     * @return function cancel : a function that cancels the binding when called
     */
    int RegisterCreatureEvent(Eluna* E)
    {
        return RegisterEntryHelper(E, Hooks::REGTYPE_CREATURE);
    }

    /**
     * Registers a [Creature] event handler for a *single* [Creature].
     *
     * <pre>
     * enum CreatureEvents
     * {
     *     CREATURE_EVENT_ON_ENTER_COMBAT                    = 1,  // (event, creature, target) - Can return true to stop normal action
     *     CREATURE_EVENT_ON_LEAVE_COMBAT                    = 2,  // (event, creature) - Can return true to stop normal action
     *     CREATURE_EVENT_ON_TARGET_DIED                     = 3,  // (event, creature, victim) - Can return true to stop normal action
     *     CREATURE_EVENT_ON_DIED                            = 4,  // (event, creature, killer) - Can return true to stop normal action
     *     CREATURE_EVENT_ON_SPAWN                           = 5,  // (event, creature) - Can return true to stop normal action
     *     CREATURE_EVENT_ON_REACH_WP                        = 6,  // (event, creature, type, id) - Can return true to stop normal action
     *     CREATURE_EVENT_ON_AIUPDATE                        = 7,  // (event, creature, diff) - Can return true to stop normal action
     *     CREATURE_EVENT_ON_RECEIVE_EMOTE                   = 8,  // (event, creature, player, emoteid) - Can return true to stop normal action
     *     CREATURE_EVENT_ON_DAMAGE_TAKEN                    = 9,  // (event, creature, attacker, damage) - Can return true to stop normal action, can return new damage as second return value.
     *     CREATURE_EVENT_ON_PRE_COMBAT                      = 10, // (event, creature, target) - Can return true to stop normal action
     *     // UNUSED
     *     CREATURE_EVENT_ON_OWNER_ATTACKED                  = 12, // (event, creature, target) - Can return true to stop normal action            // Not on mangos
     *     CREATURE_EVENT_ON_OWNER_ATTACKED_AT               = 13, // (event, creature, attacker) - Can return true to stop normal action          // Not on mangos
     *     CREATURE_EVENT_ON_HIT_BY_SPELL                    = 14, // (event, creature, caster, spellid) - Can return true to stop normal action
     *     CREATURE_EVENT_ON_SPELL_HIT_TARGET                = 15, // (event, creature, target, spellid) - Can return true to stop normal action
     *     // UNUSED                                         = 16, // (event, creature)
     *     // UNUSED                                         = 17, // (event, creature)
     *     // UNUSED                                         = 18, // (event, creature)
     *     CREATURE_EVENT_ON_JUST_SUMMONED_CREATURE          = 19, // (event, creature, summon) - Can return true to stop normal action
     *     CREATURE_EVENT_ON_SUMMONED_CREATURE_DESPAWN       = 20, // (event, creature, summon) - Can return true to stop normal action
     *     CREATURE_EVENT_ON_SUMMONED_CREATURE_DIED          = 21, // (event, creature, summon, killer) - Can return true to stop normal action    // Not on mangos
     *     CREATURE_EVENT_ON_SUMMONED                        = 22, // (event, creature, summoner) - Can return true to stop normal action
     *     CREATURE_EVENT_ON_RESET                           = 23, // (event, creature)
     *     CREATURE_EVENT_ON_REACH_HOME                      = 24, // (event, creature) - Can return true to stop normal action
     *     // UNUSED                                         = 25, // (event, creature)
     *     CREATURE_EVENT_ON_CORPSE_REMOVED                  = 26, // (event, creature, respawndelay) - Can return true to stop normal action, can return new respawndelay as second return value
     *     CREATURE_EVENT_ON_MOVE_IN_LOS                     = 27, // (event, creature, unit) - Can return true to stop normal action. Does not actually check LOS, just uses the sight range
     *     // UNUSED                                         = 28, // (event, creature)
     *     // UNUSED                                         = 29, // (event, creature)
     *     CREATURE_EVENT_ON_DUMMY_EFFECT                    = 30, // (event, caster, spellid, effindex, creature)
     *     CREATURE_EVENT_ON_QUEST_ACCEPT                    = 31, // (event, player, creature, quest) - Can return true
     *     // UNUSED                                         = 32, // (event, creature)
     *     // UNUSED                                         = 33, // (event, creature)
     *     CREATURE_EVENT_ON_QUEST_REWARD                    = 34, // (event, player, creature, quest, opt) - Can return true
     *     CREATURE_EVENT_ON_DIALOG_STATUS                   = 35, // (event, player, creature)
     *     CREATURE_EVENT_ON_ADD                             = 36, // (event, creature)
     *     CREATURE_EVENT_ON_REMOVE                          = 37, // (event, creature)
     *     CREATURE_EVENT_COUNT
     * };
     * </pre>
     *
     * @proto cancel = (guid, instance_id, event, function)
     * @proto cancel = (guid, instance_id, event, function, shots)
     *
     * @param ObjectGuid guid : the GUID of a single [Creature]
     * @param uint32 instance_id : the instance ID of a single [Creature]
     * @param uint32 event : refer to CreatureEvents above
     * @param function function : function that will be called when the event occurs
     * @param uint32 shots = 0 : the number of times the function will be called, 0 means "always call this function"
     *
     * @return function cancel : a function that cancels the binding when called
     */
    int RegisterUniqueCreatureEvent(Eluna* E)
    {
        return RegisterUniqueHelper(E, Hooks::REGTYPE_CREATURE);
    }

    /**
     * Registers a [GameObject] event handler.
     *
     * <pre>
     * enum GameObjectEvents
     * {
     *     GAMEOBJECT_EVENT_ON_AIUPDATE                    = 1,    // (event, go, diff)
     *     GAMEOBJECT_EVENT_ON_SPAWN                       = 2,    // (event, go)
     *     GAMEOBJECT_EVENT_ON_DUMMY_EFFECT                = 3,    // (event, caster, spellid, effindex, go) - Can return true to stop normal action
     *     GAMEOBJECT_EVENT_ON_QUEST_ACCEPT                = 4,    // (event, player, go, quest) - Can return true to stop normal action
     *     GAMEOBJECT_EVENT_ON_QUEST_REWARD                = 5,    // (event, player, go, quest, opt) - Can return true to stop normal action
     *     GAMEOBJECT_EVENT_ON_DIALOG_STATUS               = 6,    // (event, player, go)
     *     GAMEOBJECT_EVENT_ON_DESTROYED                   = 7,    // (event, go, attacker)
     *     GAMEOBJECT_EVENT_ON_DAMAGED                     = 8,    // (event, go, attacker)
     *     GAMEOBJECT_EVENT_ON_LOOT_STATE_CHANGE           = 9,    // (event, go, state)
     *     GAMEOBJECT_EVENT_ON_GO_STATE_CHANGED            = 10,   // (event, go, state)
     *     // UNUSED                                       = 11,   // (event, gameobject)
     *     GAMEOBJECT_EVENT_ON_ADD                         = 12,   // (event, gameobject)
     *     GAMEOBJECT_EVENT_ON_REMOVE                      = 13,   // (event, gameobject)
     *     GAMEOBJECT_EVENT_ON_USE                         = 14,   // (event, go, player) - Can return true to stop normal action
     *     GAMEOBJECT_EVENT_COUNT
     * };
     * </pre>
     *
     * @proto cancel = (entry, event, function)
     * @proto cancel = (entry, event, function, shots)
     *
     * @param uint32 entry : [GameObject] entry Id
     * @param uint32 event : [GameObject] event Id, refer to GameObjectEvents above
     * @param function function : function to register
     * @param uint32 shots = 0 : the number of times the function will be called, 0 means "always call this function"
     *
     * @return function cancel : a function that cancels the binding when called
     */
    int RegisterGameObjectEvent(Eluna* E)
    {
        return RegisterEntryHelper(E, Hooks::REGTYPE_GAMEOBJECT);
    }

    /**
     * Reloads the Lua engine.
     */
    int ReloadEluna(Eluna* E)
    {
        E->ReloadEluna();
        return 0;
    }

    /**
     * Runs a command.
     *
     * @param string command : the command to run
     */
    int RunCommand(Eluna* E)
    {
        const char* command = Eluna::CHECKVAL<const char*>(E->L, 1);
        // ignores output of the command
#ifdef CATA
        eWorld->QueueCliCommand(new CliCommandHolder(nullptr, command, nullptr, [](void*, bool) {}));
#else
        eWorld->QueueCliCommand(new CliCommandHolder(nullptr, command, [](void*, std::string_view) {}, [](void*, bool) {}));
#endif
        return 0;
    }

    /**
     * Sends a message to all [Player]s online.
     *
     * @param string message : message to send
     */
    int SendWorldMessage(Eluna* E)
    {
        const char* message = Eluna::CHECKVAL<const char*>(E->L, 1);
        eWorld->SendServerMessage(SERVER_MSG_STRING, message);
        return 0;
    }

    /**
     * Executes a SQL query on the world database and returns an [ElunaQuery].
     *
     * The query is always executed synchronously
     *   (i.e. execution halts until the query has finished and then results are returned).
     *
     *     local Q = WorldDBQuery("SELECT entry, name FROM creature_template LIMIT 10")
     *     if Q then
     *         repeat
     *             local entry, name = Q:GetUInt32(0), Q:GetString(1)
     *             print(entry, name)
     *         until not Q:NextRow()
     *     end
     *
     * @param string sql : query to execute
     * @return [ElunaQuery] results or nil if no rows found or nil if no rows found
     */
    int WorldDBQuery(Eluna* E)
    {
        const char* query = Eluna::CHECKVAL<const char*>(E->L, 1);

        ElunaQuery result = WorldDatabase.Query(query);
        if (result)
            E->Push(new ElunaQuery(result));
        else
            E->Push(E->L);

        return 1;
    }

    /**
     * Executes a SQL query on the world database.
     *
     * The query may be executed *asynchronously* (at a later, unpredictable time).
     * If you need to execute the query synchronously, use [Global:WorldDBQuery] instead.
     *
     * Any results produced are ignored.
     * If you need results from the query, use [Global:WorldDBQuery] instead.
     *
     *     WorldDBExecute("DELETE FROM my_table")
     *
     * @param string sql : query to execute
     */
    int WorldDBExecute(Eluna* E)
    {
        const char* query = Eluna::CHECKVAL<const char*>(E->L, 1);
        WorldDatabase.Execute(query);
        return 0;
    }


    /**
     * Initiates an asynchronous SQL query on the world database with a callback function.
     *
     * The query is executed asynchronously, and the provided Lua function is called when the query completes.
     * The callback function parameter is the query result (an [ElunaQuery] or nil if no rows found).
     *
     * Example usage:
     *     WorldDBQueryAsync("SELECT entry, name FROM creature_template LIMIT 10", function(results)
     *         if results then
     *             repeat
     *                 local entry, name = results:GetUInt32(0), results:GetString(1)
     *                 print(entry, name)
     *             until not results:NextRow()
     *         end
     *     end)
     *
     * @param string sql : query to execute asynchronously
     * @param function callback : the callback function to be called with the query results
     */
    int WorldDBQueryAsync(Eluna* E)
    {
        const char* query = Eluna::CHECKVAL<const char*>(E->L, 1);
        luaL_checktype(E->L, 2, LUA_TFUNCTION);

        // Push the Lua function onto the stack and create a reference
        lua_pushvalue(E->L, 2);
        int funcRef = luaL_ref(E->L, LUA_REGISTRYINDEX);

        // Validate the function reference
        if (funcRef == LUA_REFNIL || funcRef == LUA_NOREF)
        {
            luaL_argerror(E->L, 2, "unable to make a ref to function");
            return 0;
        }

        // Add an asynchronous query callback
        E->GetQueryProcessor().AddCallback(WorldDatabase.AsyncQuery(query).WithCallback([E, funcRef](QueryResult result)
        {
            ElunaQuery* eq = result ? new ElunaQuery(result) : nullptr;

            // Get the Lua function from the registry
            lua_rawgeti(E->L, LUA_REGISTRYINDEX, funcRef);

            // Push the query results as a parameter
            E->Push(eq);

            // Call the Lua function
            E->ExecuteCall(1, 0);

            // Unreference the Lua function
            luaL_unref(E->L, LUA_REGISTRYINDEX, funcRef);
        }));
        return 0;
    }

    /**
     * Executes a SQL query on the character database and returns an [ElunaQuery].
     *
     * The query is always executed synchronously
     *   (i.e. execution halts until the query has finished and then results are returned).
     *
     * For an example see [Global:WorldDBQuery].
     *
     * @param string sql : query to execute
     * @return [ElunaQuery] results or nil if no rows found
     */
    int CharDBQuery(Eluna* E)
    {
        const char* query = Eluna::CHECKVAL<const char*>(E->L, 1);

        QueryResult result = CharacterDatabase.Query(query);
        if (result)
            E->Push(new QueryResult(result));
        else
            E->Push(E->L);

        return 1;
    }

    /**
     * Executes a SQL query on the character database.
     *
     * The query may be executed *asynchronously* (at a later, unpredictable time).
     * If you need to execute the query synchronously, use [Global:CharDBQuery] instead.
     *
     * Any results produced are ignored.
     * If you need results from the query, use [Global:CharDBQuery] instead.
     *
     *     CharDBExecute("DELETE FROM my_table")
     *
     * @param string sql : query to execute
     */
    int CharDBExecute(Eluna* E)
    {
        const char* query = Eluna::CHECKVAL<const char*>(E->L, 1);
        CharacterDatabase.Execute(query);
        return 0;
    }

    /**
     * Initiates an asynchronous SQL query on the character database with a callback function.
     *
     * The query is executed asynchronously, and the provided Lua function is called when the query completes.
     * The callback function parameter is the query result (an [ElunaQuery] or nil if no rows found).
     *
     * For an example see [Global:WorldDBQueryAsync].
     *
     * @param string sql : query to execute asynchronously
     * @param function callback : the callback function to be called with the query results
     */
    int CharDBQueryAsync(Eluna* E)
    {
        const char* query = Eluna::CHECKVAL<const char*>(E->L, 1);
        luaL_checktype(E->L, 2, LUA_TFUNCTION);

        // Push the Lua function onto the stack and create a reference
        lua_pushvalue(E->L, 2);
        int funcRef = luaL_ref(E->L, LUA_REGISTRYINDEX);

        // Validate the function reference
        if (funcRef == LUA_REFNIL || funcRef == LUA_NOREF)
        {
            luaL_argerror(E->L, 2, "unable to make a ref to function");
            return 0;
        }

        // Add an asynchronous query callback
        E->GetQueryProcessor().AddCallback(CharacterDatabase.AsyncQuery(query).WithCallback([E, funcRef](QueryResult result)
        {
            ElunaQuery* eq = result ? new ElunaQuery(result) : nullptr;

            // Get the Lua function from the registry
            lua_rawgeti(E->L, LUA_REGISTRYINDEX, funcRef);

            // Push the query results as a parameter
            E->Push(eq);

            // Call the Lua function
            E->ExecuteCall(1, 0);

            // Unreference the Lua function
            luaL_unref(E->L, LUA_REGISTRYINDEX, funcRef);
        }));
        return 0;
    }

    /**
     * Executes a SQL query on the login database and returns an [ElunaQuery].
     *
     * The query is always executed synchronously
     *   (i.e. execution halts until the query has finished and then results are returned).
     *
     * For an example see [Global:WorldDBQuery].
     *
     * @param string sql : query to execute
     * @return [ElunaQuery] results or nil if no rows found
     */
    int AuthDBQuery(Eluna* E)
    {
        const char* query = Eluna::CHECKVAL<const char*>(E->L, 1);

        QueryResult result = LoginDatabase.Query(query);
        if (result)
            E->Push(new QueryResult(result));
        else
            E->Push(E->L);

        return 1;
    }

    /**
     * Executes a SQL query on the login database.
     *
     * The query may be executed *asynchronously* (at a later, unpredictable time).
     * If you need to execute the query synchronously, use [Global:AuthDBQuery] instead.
     *
     * Any results produced are ignored.
     * If you need results from the query, use [Global:AuthDBQuery] instead.
     *
     *     AuthDBExecute("DELETE FROM my_table")
     *
     * @param string sql : query to execute
     */
    int AuthDBExecute(Eluna* E)
    {
        const char* query = Eluna::CHECKVAL<const char*>(E->L, 1);
        LoginDatabase.Execute(query);
        return 0;
    }

    /**
     * Initiates an asynchronous SQL query on the login database with a callback function.
     *
     * The query is executed asynchronously, and the provided Lua function is called when the query completes.
     * The callback function parameter is the query result (an [ElunaQuery] or nil if no rows found).
     *
     * For an example see [Global:WorldDBQueryAsync].
     *
     * @param string sql : query to execute asynchronously
     * @param function callback : the callback function to be called with the query results
     */
    int AuthDBQueryAsync(Eluna* E)
    {
        const char* query = Eluna::CHECKVAL<const char*>(E->L, 1);
        luaL_checktype(E->L, 2, LUA_TFUNCTION);

        // Push the Lua function onto the stack and create a reference
        lua_pushvalue(E->L, 2);
        int funcRef = luaL_ref(E->L, LUA_REGISTRYINDEX);

        // Validate the function reference
        if (funcRef == LUA_REFNIL || funcRef == LUA_NOREF)
        {
            luaL_argerror(E->L, 2, "unable to make a ref to function");
            return 0;
        }

        // Add an asynchronous query callback
        E->GetQueryProcessor().AddCallback(LoginDatabase.AsyncQuery(query).WithCallback([E, funcRef](QueryResult result)
        {
            ElunaQuery* eq = result ? new ElunaQuery(result) : nullptr;

            // Get the Lua function from the registry
            lua_rawgeti(E->L, LUA_REGISTRYINDEX, funcRef);

            // Push the query results as a parameter
            E->Push(eq);

            // Call the Lua function
            E->ExecuteCall(1, 0);

            // Unreference the Lua function
            luaL_unref(E->L, LUA_REGISTRYINDEX, funcRef);
        }));
        return 0;
    }

    /**
     * Registers a global timed event.
     *
     * When the passed function is called, the parameters `(eventId, delay, repeats)` are passed to it.
     *
     * Repeats will decrease on each call if the event does not repeat indefinitely
     *
     * @proto eventId = (function, delay)
     * @proto eventId = (function, delaytable)
     * @proto eventId = (function, delay, repeats)
     * @proto eventId = (function, delaytable, repeats)
     *
     * @param function function : function to trigger when the time has passed
     * @param uint32 delay : set time in milliseconds for the event to trigger
     * @param table delaytable : a table `{min, max}` containing the minimum and maximum delay time
     * @param uint32 repeats = 1 : how many times for the event to repeat, 0 is infinite
     * @return int eventId : unique ID for the timed event used to cancel it or nil
     */
    int CreateLuaEvent(Eluna* E)
    {
        luaL_checktype(E->L, 1, LUA_TFUNCTION);
        uint32 min, max;
        if (lua_istable(E->L, 2))
        {
            E->Push(1);
            lua_gettable(E->L, 2);
            min = Eluna::CHECKVAL<uint32>(E->L, -1);
            E->Push(2);
            lua_gettable(E->L, 2);
            max = Eluna::CHECKVAL<uint32>(E->L, -1);
            lua_pop(E->L, 2);
        }
        else
            min = max = Eluna::CHECKVAL<uint32>(E->L, 2);
        uint32 repeats = Eluna::CHECKVAL<uint32>(E->L, 3, 1);

        if (min > max)
            return luaL_argerror(E->L, 2, "min is bigger than max delay");

        lua_pushvalue(E->L, 1);
        int functionRef = luaL_ref(E->L, LUA_REGISTRYINDEX);
        if (functionRef != LUA_REFNIL && functionRef != LUA_NOREF)
        {
            E->eventMgr->globalProcessor->AddEvent(functionRef, min, max, repeats);
            E->Push(functionRef);
        }
        return 1;
    }

    /**
     * Removes a global timed event specified by ID.
     *
     * @param int eventId : event Id to remove
     * @param bool all_Events = false : remove from all events, not just global
     */
    int RemoveEventById(Eluna* E)
    {
        int eventId = Eluna::CHECKVAL<int>(E->L, 1);
        bool all_Events = Eluna::CHECKVAL<bool>(E->L, 1, false);

        // not thread safe
        if (all_Events)
            E->eventMgr->SetState(eventId, LUAEVENT_STATE_ABORT);
        else
            E->eventMgr->globalProcessor->SetState(eventId, LUAEVENT_STATE_ABORT);
        return 0;
    }

    /**
     * Removes all global timed events.
     *
     * @param bool all_Events = false : remove all events, not just global
     */
    int RemoveEvents(Eluna* E)
    {
        bool all_Events = Eluna::CHECKVAL<bool>(E->L, 1, false);

        // not thread safe
        if (all_Events)
            E->eventMgr->SetStates(LUAEVENT_STATE_ABORT);
        else
            E->eventMgr->globalProcessor->SetStates(LUAEVENT_STATE_ABORT);
        return 0;
    }

    /**
     * Performs an in-game spawn and returns the [Creature] or [GameObject] spawned.
     *
     * @param int32 spawnType : type of object to spawn, 1 = [Creature], 2 = [GameObject]
     * @param uint32 entry : entry ID of the [Creature] or [GameObject]
     * @param uint32 mapId : map ID to spawn the [Creature] or [GameObject] in
     * @param uint32 instanceId : instance ID to put the [Creature] or [GameObject] in. Non instance is 0
     * @param float x : x coordinate of the [Creature] or [GameObject]
     * @param float y : y coordinate of the [Creature] or [GameObject]
     * @param float z : z coordinate of the [Creature] or [GameObject]
     * @param float o : o facing/orientation of the [Creature] or [GameObject]
     * @param bool save = false : optional to save the [Creature] or [GameObject] to the database
     * @param uint32 durorresptime = 0 : despawn time of the [Creature] if it's not saved or respawn time of [GameObject]
     * @param uint32 phase = 1 : phase to put the [Creature] or [GameObject] in
     * @return [WorldObject] worldObject : returns [Creature] or [GameObject]
     */
    int PerformIngameSpawn(Eluna* E)
    {
        int spawntype = Eluna::CHECKVAL<int>(E->L, 1);
        uint32 entry = Eluna::CHECKVAL<uint32>(E->L, 2);
        uint32 mapID = Eluna::CHECKVAL<uint32>(E->L, 3);
        uint32 instanceID = Eluna::CHECKVAL<uint32>(E->L, 4);
        float x = Eluna::CHECKVAL<float>(E->L, 5);
        float y = Eluna::CHECKVAL<float>(E->L, 6);
        float z = Eluna::CHECKVAL<float>(E->L, 7);
        float o = Eluna::CHECKVAL<float>(E->L, 8);
        bool save = Eluna::CHECKVAL<bool>(E->L, 9, false);
        uint32 durorresptime = Eluna::CHECKVAL<uint32>(E->L, 10, 0);
        uint32 phase = Eluna::CHECKVAL<uint32>(E->L, 11, PHASEMASK_NORMAL);

        if (!phase)
        {
            E->Push(E->L);
            return 1;
        }

        Map* map = eMapMgr->FindMap(mapID, instanceID);
        if (!map)
        {
            E->Push(E->L);
            return 1;
        }

        Position pos = { x, y, z, o };

        if (spawntype == 1) // spawn creature
        {
            if (save)
            {
                Creature* creature = new Creature();
#ifdef CATA
                if (!creature->Create(map->GenerateLowGuid<HighGuid::Unit>(), map, entry, pos))
#else
                if (!creature->Create(map->GenerateLowGuid<HighGuid::Unit>(), map, phase, entry, pos))
#endif
                {
                    delete creature;
                    E->Push(E->L);
                    return 1;
                }

#ifdef CATA
                creature->SaveToDB(map->GetId(), (1 << map->GetSpawnMode()));
#else
                creature->SaveToDB(map->GetId(), (1 << map->GetSpawnMode()), phase);
#endif

                uint32 db_guid = creature->GetSpawnId();

                // To call _LoadGoods(); _LoadQuests(); CreateTrainerSpells()
                // current "creature" variable is deleted and created fresh new, otherwise old values might trigger asserts or cause undefined behavior
                creature->CleanupsBeforeDelete();
                delete creature;
                creature = new Creature();

                if (!creature->LoadFromDB(db_guid, map, true, true))
                {
                    delete creature;
                    E->Push(E->L);
                    return 1;
                }

                eObjectMgr->AddCreatureToGrid(db_guid, eObjectMgr->GetCreatureData(db_guid));
                E->Push(creature);
            }
            else
            {
#ifdef CATA
                SummonCreatureExtraArgs extraArgs;
                extraArgs.SummonDuration = durorresptime;
                TempSummon* creature = map->SummonCreature(entry, pos, extraArgs);
#else
                TempSummon* creature = map->SummonCreature(entry, pos, NULL, durorresptime);
#endif
                if (!creature)
                {
                    E->Push(E->L);
                    return 1;
                }

                if (durorresptime)
                    creature->SetTempSummonType(TEMPSUMMON_TIMED_OR_DEAD_DESPAWN);
                else
                    creature->SetTempSummonType(TEMPSUMMON_MANUAL_DESPAWN);

                E->Push(creature);
            }

            return 1;
        }

        if (spawntype == 2) // Spawn object
        {
            const GameObjectTemplate* objectInfo = eObjectMgr->GetGameObjectTemplate(entry);
            if (!objectInfo)
            {
                E->Push(E->L);
                return 1;
            }

            if (objectInfo->displayId && !sGameObjectDisplayInfoStore.LookupEntry(objectInfo->displayId))
            {
                E->Push(E->L);
                return 1;
            }

            GameObject* object = new GameObject;
            uint32 guidLow = map->GenerateLowGuid<HighGuid::GameObject>();
            QuaternionData rot = QuaternionData::fromEulerAnglesZYX(o, 0.f, 0.f);
#ifdef CATA
            if (!object->Create(guidLow, objectInfo->entry, map, Position(x, y, z, o), rot, 0, GO_STATE_READY))
#else
            if (!object->Create(guidLow, objectInfo->entry, map, phase, Position(x, y, z, o), rot, 0, GO_STATE_READY))
#endif
            {
                delete object;
                E->Push(E->L);
                return 1;
            }

            if (durorresptime)
                object->SetRespawnTime(durorresptime);

            if (save)
            {
                // fill the gameobject data and save to the db
#ifdef CATA
                object->SaveToDB(map->GetId(), (1 << map->GetSpawnMode()));
#else
                object->SaveToDB(map->GetId(), (1 << map->GetSpawnMode()), phase);
#endif
                guidLow = object->GetSpawnId();

                // delete the old object and do a clean load from DB with a fresh new GameObject instance.
                // this is required to avoid weird behavior and memory leaks
                delete object;

                object = new GameObject();
                // this will generate a new lowguid if the object is in an instance
                if (!object->LoadFromDB(guidLow, map, true))
                {
                    delete object;
                    E->Push(E->L);
                    return 1;
                }
                eObjectMgr->AddGameobjectToGrid(guidLow, eObjectMgr->GetGameObjectData(guidLow));
            }
            else
                map->AddToMap(object);
            E->Push(object);
            return 1;
        }

        E->Push(E->L);
        return 1;
    }

    /**
     * Creates a [WorldPacket].
     *
     * @param [Opcodes] opcode : the opcode of the packet
     * @param uint32 size : the size of the packet
     * @return [WorldPacket] packet
     */
    int CreatePacket(Eluna* E)
    {
        uint32 opcode = Eluna::CHECKVAL<uint32>(E->L, 1);
        size_t size = Eluna::CHECKVAL<size_t>(E->L, 2);
        if (opcode >= NUM_MSG_TYPES)
            return luaL_argerror(E->L, 1, "valid opcode expected");

        E->Push(new WorldPacket((OpcodesList)opcode, size));
        return 1;
    }

    /**
     * Adds an [Item] to a vendor and updates the world database.
     *
     * @param uint32 entry : [Creature] entry Id
     * @param uint32 item : [Item] entry Id
     * @param int32 maxcount : max [Item] stack count
     * @param uint32 incrtime : combined with maxcount, incrtime tells how often (in seconds) the vendor list is refreshed and the limited [Item] copies are restocked
     * @param uint32 extendedcost : unique cost of an [Item], such as conquest points for example
     */
    int AddVendorItem(Eluna* E)
    {
        uint32 entry = Eluna::CHECKVAL<uint32>(E->L, 1);
        uint32 item = Eluna::CHECKVAL<uint32>(E->L, 2);
        int maxcount = Eluna::CHECKVAL<int>(E->L, 3);
        uint32 incrtime = Eluna::CHECKVAL<uint32>(E->L, 4);
        uint32 extendedcost = Eluna::CHECKVAL<uint32>(E->L, 5);

#ifdef CATA
        VendorItem vItem;
        vItem.item = item;
        vItem.maxcount = maxcount;
        vItem.incrtime = incrtime;
        vItem.ExtendedCost = extendedcost;

        if (!eObjectMgr->IsVendorItemValid(entry, vItem))
            return 0;
        eObjectMgr->AddVendorItem(entry, vItem);
#else
        if (!eObjectMgr->IsVendorItemValid(entry, item, maxcount, incrtime, extendedcost))
            return 0;

        eObjectMgr->AddVendorItem(entry, item, maxcount, incrtime, extendedcost);
#endif
        return 0;
    }

    /**
     * Removes an [Item] from a vendor and updates the database.
     *
     * @param uint32 entry : [Creature] entry Id
     * @param uint32 item : [Item] entry Id
     */
    int VendorRemoveItem(Eluna* E)
    {
        uint32 entry = Eluna::CHECKVAL<uint32>(E->L, 1);
        uint32 item = Eluna::CHECKVAL<uint32>(E->L, 2);
        if (!eObjectMgr->GetCreatureTemplate(entry))
            return luaL_argerror(E->L, 1, "valid CreatureEntry expected");

#ifdef CATA
        eObjectMgr->RemoveVendorItem(entry, item, 1);
#else
        eObjectMgr->RemoveVendorItem(entry, item);
#endif

        return 0;
    }

    /**
     * Removes all [Item]s from a vendor and updates the database.
     *
     * @param uint32 entry : [Creature] entry Id
     */
    int VendorRemoveAllItems(Eluna* E)
    {
        uint32 entry = Eluna::CHECKVAL<uint32>(E->L, 1);

        VendorItemData const* items = eObjectMgr->GetNpcVendorItemList(entry);
        if (!items || items->Empty())
            return 0;

        auto const itemlist = items->m_items;
        for (auto itr = itemlist.begin(); itr != itemlist.end(); ++itr)
#ifdef CATA
            eObjectMgr->RemoveVendorItem(entry, itr->item, 1);
#else
            eObjectMgr->RemoveVendorItem(entry, itr->item);

#endif
        return 0;
    }

    /**
     * Kicks a [Player] from the server.
     *
     * @param [Player] player : [Player] to kick
     */
    int Kick(Eluna* E)
    {
        Player* player = Eluna::CHECKOBJ<Player>(E->L, 1);

#ifndef CATA
        player->GetSession()->KickPlayer("GlobalMethods::Kick Kick the player");
#else
        player->GetSession()->KickPlayer();
#endif
        return 0;
    }

    /**
     * Ban's a [Player]'s account, character or IP
     *
     *     enum BanMode
     *     {
     *         BAN_ACCOUNT = 0,
     *         BAN_CHARACTER = 1,
     *         BAN_IP = 2
     *     };
     *
     * @param [BanMode] banMode : method of ban, refer to BanMode above
     * @param string nameOrIP : If BanMode is 0 then accountname, if 1 then charactername if 2 then ip
     * @param uint32 duration : duration (in seconds) of the ban
     * @param string reason = "" : ban reason, this is optional
     * @param string whoBanned = "" : the [Player]'s name that banned the account, character or IP, this is optional
     * @return int result : status of the ban. 0 if success, 1 if syntax error, 2 if target not found, 3 if a longer ban already exists, nil if unknown result
     */
    int Ban(Eluna* E)
    {
        int banMode = Eluna::CHECKVAL<int>(E->L, 1);
        std::string nameOrIP = Eluna::CHECKVAL<std::string>(E->L, 2);
        uint32 duration = Eluna::CHECKVAL<uint32>(E->L, 3);
        const char* reason = Eluna::CHECKVAL<const char*>(E->L, 4, "");
        const char* whoBanned = Eluna::CHECKVAL<const char*>(E->L, 5, "");

        const int BAN_ACCOUNT = 0;
        const int BAN_CHARACTER = 1;
        const int BAN_IP = 2;

        BanMode mode = BanMode::BAN_ACCOUNT;

        switch (banMode)
        {
            case BAN_ACCOUNT:
                if (!Utf8ToUpperOnlyLatin(nameOrIP))
                    return luaL_argerror(E->L, 2, "invalid account name");
                mode = BanMode::BAN_ACCOUNT;
                break;
            case BAN_CHARACTER:
                if (!normalizePlayerName(nameOrIP))
                    return luaL_argerror(E->L, 2, "invalid character name");
                mode = BanMode::BAN_CHARACTER;
                break;
            case BAN_IP:
                if (!IsIPAddress(nameOrIP.c_str()))
                    return luaL_argerror(E->L, 2, "invalid ip");
                mode = BanMode::BAN_IP;
                break;
            default:
                return luaL_argerror(E->L, 1, "unknown banmode");
        }

        BanReturn result;
        result = eWorld->BanAccount(mode, nameOrIP, duration, reason, whoBanned);
        switch (result)
        {
        case BanReturn::BAN_SUCCESS:
            E->Push(0);
            break;
        case BanReturn::BAN_SYNTAX_ERROR:
            E->Push(1);
            break;
        case BanReturn::BAN_NOTFOUND:
            E->Push(2);
            break;
        case BanReturn::BAN_EXISTS:
            E->Push(3);
            break;
        }
        return 1;
    }

    /**
     * Saves all [Player]s.
     */
    int SaveAllPlayers(Eluna* /*E*/)
    {
        eObjectAccessor()SaveAllPlayers();
        return 0;
    }

    /**
     * Sends mail to a [Player].
     *
     * There can be several item entry-amount pairs at the end of the function.
     * There can be maximum of 12 different items.
     *
     *     enum MailStationery
     *     {
     *         MAIL_STATIONERY_TEST = 1,
     *         MAIL_STATIONERY_DEFAULT = 41,
     *         MAIL_STATIONERY_GM = 61,
     *         MAIL_STATIONERY_AUCTION = 62,
     *         MAIL_STATIONERY_VAL = 64, // Valentine
     *         MAIL_STATIONERY_CHR = 65, // Christmas
     *         MAIL_STATIONERY_ORP = 67 // Orphan
     *     };
     *
     * @param string subject : title (subject) of the mail
     * @param string text : contents of the mail
     * @param uint32 receiverGUIDLow : low GUID of the receiver
     * @param uint32 senderGUIDLow = 0 : low GUID of the sender
     * @param [MailStationery] stationary = MAIL_STATIONERY_DEFAULT : type of mail that is being sent as, refer to MailStationery above
     * @param uint32 delay = 0 : mail send delay in milliseconds
     * @param uint32 money = 0 : money to send
     * @param uint32 cod = 0 : cod money amount
     * @param uint32 entry = 0 : entry of an [Item] to send with mail
     * @param uint32 amount = 0 : amount of the [Item] to send with mail
     * @return uint32 itemGUIDlow : low GUID of the item. Up to 12 values returned, returns nil if no further items are sent
     */
    int SendMail(Eluna* E)
    {
        int i = 0;
        std::string subject = Eluna::CHECKVAL<std::string>(E->L, ++i);
        std::string text = Eluna::CHECKVAL<std::string>(E->L, ++i);
        uint32 receiverGUIDLow = Eluna::CHECKVAL<uint32>(E->L, ++i);
        uint32 senderGUIDLow = Eluna::CHECKVAL<uint32>(E->L, ++i, 0);
        uint32 stationary = Eluna::CHECKVAL<uint32>(E->L, ++i, MAIL_STATIONERY_DEFAULT);
        uint32 delay = Eluna::CHECKVAL<uint32>(E->L, ++i, 0);
        uint32 money = Eluna::CHECKVAL<uint32>(E->L, ++i, 0);
        uint32 cod = Eluna::CHECKVAL<uint32>(E->L, ++i, 0);
        int argAmount = lua_gettop(E->L);

        MailSender sender(MAIL_NORMAL, senderGUIDLow, (MailStationery)stationary);
        MailDraft draft(subject, text);

        if (cod)
            draft.AddCOD(cod);
        if (money)
            draft.AddMoney(money);

        CharacterDatabaseTransaction trans = CharacterDatabase.BeginTransaction();
        uint8 addedItems = 0;
        while (addedItems <= MAX_MAIL_ITEMS && i + 2 <= argAmount)
        {
            uint32 entry = Eluna::CHECKVAL<uint32>(E->L, ++i);
            uint32 amount = Eluna::CHECKVAL<uint32>(E->L, ++i);

            ItemTemplate const* item_proto = eObjectMgr->GetItemTemplate(entry);
            if (!item_proto)
            {
                luaL_error(E->L, "Item entry %d does not exist", entry);
                continue;
            }
#ifdef CATA
            if (amount < 1 || (item_proto->ExtendedData->MaxCount > 0 && amount > uint32(item_proto->ExtendedData->MaxCount)))
#else
            if (amount < 1 || (item_proto->MaxCount > 0 && amount > uint32(item_proto->MaxCount)))
#endif
            {
                luaL_error(E->L, "Item entry %d has invalid amount %d", entry, amount);
                continue;
            }
            if (Item* item = Item::CreateItem(entry, amount))
            {
                item->SaveToDB(trans);
                draft.AddItem(item);
                E->Push(item->GetGUID().GetCounter());
                ++addedItems;
            }
        }

        Player* receiverPlayer = eObjectAccessor()FindPlayer(MAKE_NEW_GUID(receiverGUIDLow, 0, HIGHGUID_PLAYER));
        draft.SendMailTo(trans, MailReceiver(receiverPlayer, receiverGUIDLow), sender, MAIL_CHECK_MASK_NONE, delay);
        CharacterDatabase.CommitTransaction(trans);

        return addedItems;
    }

    /**
     * Performs a bitwise AND (a & b).
     *
     * @param uint32 a
     * @param uint32 b
     * @return uint32 result
     */
    int bit_and(Eluna* E)
    {
        uint32 a = Eluna::CHECKVAL<uint32>(E->L, 1);
        uint32 b = Eluna::CHECKVAL<uint32>(E->L, 2);
        E->Push(a & b);
        return 1;
    }

    /**
     * Performs a bitwise OR (a | b).
     *
     * @param uint32 a
     * @param uint32 b
     * @return uint32 result
     */
    int bit_or(Eluna* E)
    {
        uint32 a = Eluna::CHECKVAL<uint32>(E->L, 1);
        uint32 b = Eluna::CHECKVAL<uint32>(E->L, 2);
        E->Push(a | b);
        return 1;
    }

    /**
     * Performs a bitwise left-shift (a << b).
     *
     * @param uint32 a
     * @param uint32 b
     * @return uint32 result
     */
    int bit_lshift(Eluna* E)
    {
        uint32 a = Eluna::CHECKVAL<uint32>(E->L, 1);
        uint32 b = Eluna::CHECKVAL<uint32>(E->L, 2);
        E->Push(a << b);
        return 1;
    }

    /**
     * Performs a bitwise right-shift (a >> b).
     *
     * @param uint32 a
     * @param uint32 b
     * @return uint32 result
     */
    int bit_rshift(Eluna* E)
    {
        uint32 a = Eluna::CHECKVAL<uint32>(E->L, 1);
        uint32 b = Eluna::CHECKVAL<uint32>(E->L, 2);
        E->Push(a >> b);
        return 1;
    }

    /**
     * Performs a bitwise XOR (a ^ b).
     *
     * @param uint32 a
     * @param uint32 b
     * @return uint32 result
     */
    int bit_xor(Eluna* E)
    {
        uint32 a = Eluna::CHECKVAL<uint32>(E->L, 1);
        uint32 b = Eluna::CHECKVAL<uint32>(E->L, 2);
        E->Push(a ^ b);
        return 1;
    }

    /**
     * Performs a bitwise NOT (~a).
     *
     * @param uint32 a
     * @return uint32 result
     */
    int bit_not(Eluna* E)
    {
        uint32 a = Eluna::CHECKVAL<uint32>(E->L, 1);
        E->Push(~a);
        return 1;
    }

    /**
     * Adds a taxi path to a specified map, returns the used pathId.
     * 
     * Note that the first taxi point needs to be near the player when he starts the taxi path.
     * The function should also be used only **once** per path added so use it on server startup for example.
     *
     * Related function: [Player:StartTaxi]
     *
     *     -- Execute on startup
     *     local pathTable = {{mapid, x, y, z}, {mapid, x, y, z}}
     *     local path = AddTaxiPath(pathTable, 28135, 28135)
     *
     *     -- Execute when the player should fly
     *     player:StartTaxi(path)
     *
     * @param table waypoints : table containing waypoints: {map, x, y, z[, actionFlag, delay]}
     * @param uint32 mountA : alliance [Creature] entry
     * @param uint32 mountH : horde [Creature] entry
     * @param uint32 price = 0 : price of the taxi path
     * @param uint32 pathId = 0 : path Id of the taxi path
     * @return uint32 actualPathId
     */
    int AddTaxiPath(Eluna* E)
    {
        luaL_checktype(E->L, 1, LUA_TTABLE);
        uint32 mountA = Eluna::CHECKVAL<uint32>(E->L, 2);
        uint32 mountH = Eluna::CHECKVAL<uint32>(E->L, 3);
        uint32 price = Eluna::CHECKVAL<uint32>(E->L, 4, 0);
        uint32 pathId = Eluna::CHECKVAL<uint32>(E->L, 5, 0);
        lua_pushvalue(E->L, 1);
        // Stack: {nodes}, mountA, mountH, price, pathid, {nodes}

        std::list<TaxiPathNodeEntry> nodes;

        int start = lua_gettop(E->L);
        int end = start;

        E->Push(E->L);
        // Stack: {nodes}, mountA, mountH, price, pathid, {nodes}, nil
        while (lua_next(E->L, -2) != 0)
        {
            // Stack: {nodes}, mountA, mountH, price, pathid, {nodes}, key, value
            luaL_checktype(E->L, -1, LUA_TTABLE);
            E->Push(E->L);
            // Stack: {nodes}, mountA, mountH, price, pathid, {nodes}, key, value, nil
            while (lua_next(E->L, -2) != 0)
            {
                // Stack: {nodes}, mountA, mountH, price, pathid, {nodes}, key, value, key2, value2
                lua_insert(E->L, end++);
                // Stack: {nodes}, mountA, mountH, price, pathid, {nodes}, value2, key, value, key2
            }
            // Stack: {nodes}, mountA, mountH, price, pathid, {nodes}, value2, key, value
            if (start == end)
                continue;
            if (end - start < 4) // no mandatory args, dont add
                return luaL_argerror(E->L, 1, "all waypoints do not have mandatory arguments");

            while (end - start < 8) // fill optional args with 0
            {
                E->Push(0);
                lua_insert(E->L, end++);
                // Stack: {nodes}, mountA, mountH, price, pathid, {nodes}, node, key, value
            }
            TaxiPathNodeEntry entry;

            // mandatory
            entry.ContinentID = Eluna::CHECKVAL<uint32>(E->L, start);
            entry.Loc.X = Eluna::CHECKVAL<float>(E->L, start + 1);
            entry.Loc.Y = Eluna::CHECKVAL<float>(E->L, start + 2);
            entry.Loc.Z = Eluna::CHECKVAL<float>(E->L, start + 3);
            // optional
            entry.Flags = Eluna::CHECKVAL<uint32>(E->L, start + 4, 0);
            entry.Delay = Eluna::CHECKVAL<uint32>(E->L, start + 5, 0);

            nodes.push_back(entry);

            while (end != start) // remove args
                if (!lua_isnone(E->L, --end))
                    lua_remove(E->L, end);
            // Stack: {nodes}, mountA, mountH, price, pathid, {nodes}, key, value

            lua_pop(E->L, 1);
            // Stack: {nodes}, mountA, mountH, price, pathid, {nodes}, key
        }
        // Stack: {nodes}, mountA, mountH, price, pathid, {nodes}
        lua_pop(E->L, 1);
        // Stack: {nodes}, mountA, mountH, price, pathid

        if (nodes.size() < 2)
            return 1;
        if (!pathId)
            pathId = sTaxiPathNodesByPath.size();
        if (sTaxiPathNodesByPath.size() <= pathId)
            sTaxiPathNodesByPath.resize(pathId + 1);
        sTaxiPathNodesByPath[pathId].clear();
        sTaxiPathNodesByPath[pathId].resize(nodes.size());
        static uint32 nodeId = 500;
        uint32 startNode = nodeId;
        uint32 index = 0;
        for (std::list<TaxiPathNodeEntry>::iterator it = nodes.begin(); it != nodes.end(); ++it)
        {
            TaxiPathNodeEntry& entry = *it;
            TaxiNodesEntry* nodeEntry = new TaxiNodesEntry();

            entry.PathID = pathId;
            entry.NodeIndex = nodeId;
            nodeEntry->ID = index;
            nodeEntry->ContinentID = entry.ContinentID;
            nodeEntry->Pos.X = entry.Loc.X;
            nodeEntry->Pos.Y = entry.Loc.Y;
            nodeEntry->Pos.Z = entry.Loc.Z;
            nodeEntry->MountCreatureID[0] = mountH;
            nodeEntry->MountCreatureID[1] = mountA;
            sTaxiNodesStore.SetEntry(nodeId++, nodeEntry);
            sTaxiPathNodesByPath[pathId][index++] = new TaxiPathNodeEntry(entry);
        }
        if (startNode >= nodeId)
            return 1;
        sTaxiPathSetBySource[startNode][nodeId - 1] = TaxiPathBySourceAndDestination(pathId, price);
        TaxiPathEntry* pathEntry = new TaxiPathEntry();

        pathEntry->FromTaxiNode = startNode;
        pathEntry->ToTaxiNode = nodeId - 1;
        pathEntry->Cost = price;
        pathEntry->ID = pathId;

        sTaxiPathStore.SetEntry(pathId, pathEntry);

        E->Push(pathId);
        return 1;
    }
    /**
     * Returns `true` if Eluna is in compatibility mode, `false` if in multistate.
     *
     * @return bool isCompatibilityMode
     */
    int IsCompatibilityMode(Eluna* E)
    {
        E->Push(E->GetCompatibilityMode());
        return 1;
    }

    /**
     * Returns `true` if the bag and slot is a valid inventory position, otherwise `false`.
     *
     * Some commonly used combinations:
     *
     * *Bag 255 (common character inventory)*
     *
     * - Slots 0-18: equipment
     * - Slots 19-22: bag slots
     * - Slots 23-38: backpack
     * - Slots 39-66: bank main slots
     * - Slots 67-74: bank bag slots
     * - Slots 86-117: keyring
     *
     * *Bags 19-22 (equipped bags)*
     *
     * - Slots 0-35
     *
     * *Bags 67-74 (bank bags)*
     *
     * - Slots 0-35
     *
     * @param uint8 bag : the bag the [Item] is in, you can get this with [Item:GetBagSlot]
     * @param uint8 slot : the slot the [Item] is in within the bag, you can get this with [Item:GetSlot]
     * @return bool isInventoryPos
     */
    int IsInventoryPos(Eluna* E)
    {
        uint8 bag = Eluna::CHECKVAL<uint8>(E->L, 1);
        uint8 slot = Eluna::CHECKVAL<uint8>(E->L, 2);

        E->Push(Player::IsInventoryPos(bag, slot));
        return 1;
    }

    /**
     * Returns `true` if the bag and slot is a valid equipment position, otherwise `false`.
     *
     * See [Global:IsInventoryPos] for bag/slot combination examples.
     *
     * @param uint8 bag : the bag the [Item] is in, you can get this with [Item:GetBagSlot]
     * @param uint8 slot : the slot the [Item] is in within the bag, you can get this with [Item:GetSlot]
     * @return bool isEquipmentPosition
     */
    int IsEquipmentPos(Eluna* E)
    {
        uint8 bag = Eluna::CHECKVAL<uint8>(E->L, 1);
        uint8 slot = Eluna::CHECKVAL<uint8>(E->L, 2);

        E->Push(Player::IsEquipmentPos(bag, slot));
        return 1;
    }

    /**
     * Returns `true` if the bag and slot is a valid bank position, otherwise `false`.
     *
     * See [Global:IsInventoryPos] for bag/slot combination examples.
     *
     * @param uint8 bag : the bag the [Item] is in, you can get this with [Item:GetBagSlot]
     * @param uint8 slot : the slot the [Item] is in within the bag, you can get this with [Item:GetSlot]
     * @return bool isBankPosition
     */
    int IsBankPos(Eluna* E)
    {
        uint8 bag = Eluna::CHECKVAL<uint8>(E->L, 1);
        uint8 slot = Eluna::CHECKVAL<uint8>(E->L, 2);

        E->Push(Player::IsBankPos(bag, slot));
        return 1;
    }

    /**
     * Returns `true` if the bag and slot is a valid bag position, otherwise `false`.
     *
     * See [Global:IsInventoryPos] for bag/slot combination examples.
     *
     * @param uint8 bag : the bag the [Item] is in, you can get this with [Item:GetBagSlot]
     * @param uint8 slot : the slot the [Item] is in within the bag, you can get this with [Item:GetSlot]
     * @return bool isBagPosition
     */
    int IsBagPos(Eluna* E)
    {
        uint8 bag = Eluna::CHECKVAL<uint8>(E->L, 1);
        uint8 slot = Eluna::CHECKVAL<uint8>(E->L, 2);

        E->Push(Player::IsBagPos((bag << 8) + slot));
        return 1;
    }

    /**
     * Returns `true` if the event is currently active, otherwise `false`.
     *
     * @param uint16 eventId : the event id to check.
     * @return bool isActive
     */
    int IsGameEventActive(Eluna* E)
    {
        uint16 eventId = Eluna::CHECKVAL<uint16>(E->L, 1);

        E->Push(eGameEventMgr->IsActiveEvent(eventId));
        return 1;
    }

    /**
     * Returns the server's current time.
     *
     * @return uint32 currTime : the current time, in milliseconds
     */
    int GetCurrTime(Eluna* E)
    {
        E->Push(ElunaUtil::GetCurrTime());
        return 1;
    }

    /**
     * Returns the difference between an old timestamp and the current time.
     *
     * @param uint32 oldTime : an old timestamp, in milliseconds
     * @return uint32 timeDiff : the difference, in milliseconds
     */
    int GetTimeDiff(Eluna* E)
    {
        uint32 oldtimems = Eluna::CHECKVAL<uint32>(E->L, 1);

        E->Push(ElunaUtil::GetTimeDiff(oldtimems));
        return 1;
    }

    static std::string GetStackAsString(Eluna* E)
    {
        std::ostringstream oss;
        int top = lua_gettop(E->L);
        for (int i = 1; i <= top; ++i)
        {
            oss << luaL_tolstring(E->L, i, NULL);
            lua_pop(E->L, 1);
        }
        return oss.str();
    }

    /**
     * Prints given parameters to the info log.
     *
     * @param ...
     */
    int PrintInfo(Eluna* E)
    {
        ELUNA_LOG_INFO("%s", GetStackAsString(E).c_str());
        return 0;
    }

    /**
     * Prints given parameters to the error log.
     *
     * @param ...
     */
    int PrintError(Eluna* E)
    {
        ELUNA_LOG_ERROR("%s", GetStackAsString(E).c_str());
        return 0;
    }

    /**
     * Prints given parameters to the debug log.
     *
     * @param ...
     */
    int PrintDebug(Eluna* E)
    {
        ELUNA_LOG_DEBUG("%s", GetStackAsString(E).c_str());
        return 0;
    }

    /**
    * Starts the event by eventId, if force is set, the event will force start regardless of previous event state.
    *
    * @param uint16 eventId : the event id to start.
    * @param bool force = false : set `true` to force start the event.
    */
    int StartGameEvent(Eluna* E)
    {
        uint16 eventId = Eluna::CHECKVAL<uint16>(E->L, 1);
        bool force = Eluna::CHECKVAL<bool>(E->L, 2, false);

        eGameEventMgr->StartEvent(eventId, force);
        return 0;
    }

    /**
    * Stops the event by eventId, if force is set, the event will force stop regardless of previous event state.
    *
    * @param uint16 eventId : the event id to stop.
    * @param bool force = false : set `true` to force stop the event.
    */
    int StopGameEvent(Eluna* E)
    {
        uint16 eventId = Eluna::CHECKVAL<uint16>(E->L, 1);
        bool force = Eluna::CHECKVAL<bool>(E->L, 2, false);

        eGameEventMgr->StopEvent(eventId, force);
        return 0;
    }

    /**
     * Returns an object representing a `long long` (64-bit) value.
     *
     * The value by default is 0, but can be initialized to a value by passing a number or long long as a string.
     *
     * @proto value = ()
     * @proto value = (n)
     * @proto value = (n_ll)
     * @proto value = (n_str)
     * @param int32 n
     * @param int64 n_ll
     * @param string n_str
     * @return int64 value
     */
    int CreateLongLong(Eluna* E)
    {
        long long init = 0;
        if (lua_isstring(E->L, 1))
        {
            std::string str = Eluna::CHECKVAL<std::string>(E->L, 1);
            std::istringstream iss(str);
            iss >> init;
            if (iss.bad())
                return luaL_argerror(E->L, 1, "long long (as string) could not be converted");
        }
        else if (!lua_isnoneornil(E->L, 1))
            init = Eluna::CHECKVAL<long long>(E->L, 1);

        E->Push(init);
        return 1;
    }

    /**
     * Returns an object representing an `unsigned long long` (64-bit) value.
     *
     * The value by default is 0, but can be initialized to a value by passing a number or unsigned long long as a string.
     *
     * @proto value = ()
     * @proto value = (n)
     * @proto value = (n_ull)
     * @proto value = (n_str)
     * @param uint32 n
     * @param uint64 n_ull
     * @param string n_str
     * @return uint64 value
     */
    int CreateULongLong(Eluna* E)
    {
        unsigned long long init = 0;
        if (lua_isstring(E->L, 1))
        {
            std::string str = Eluna::CHECKVAL<std::string>(E->L, 1);
            std::istringstream iss(str);
            iss >> init;
            if (iss.bad())
                return luaL_argerror(E->L, 1, "unsigned long long (as string) could not be converted");
        }
        else if (!lua_isnoneornil(E->L, 1))
            init = Eluna::CHECKVAL<unsigned long long>(E->L, 1);

        E->Push(init);
        return 1;
    }

    /**
     * Unbinds event handlers for either all [BattleGround] events, or one type of event.
     *
     * If `event_type` is `nil`, all [BattleGround] event handlers are cleared.
     *
     * Otherwise, only event handlers for `event_type` are cleared.
     *
     * @proto ()
     * @proto (event_type)
     * @param uint32 event_type : the event whose handlers will be cleared, see [Global:RegisterBGEvent]
     */
    int ClearBattleGroundEvents(Eluna* E)
    {
        typedef EventKey<Hooks::BGEvents> Key;

        if (lua_isnoneornil(E->L, 1))
        {
            E->BGEventBindings->Clear();
        }
        else
        {
            uint32 event_type = Eluna::CHECKVAL<uint32>(E->L, 1);
            E->BGEventBindings->Clear(Key((Hooks::BGEvents)event_type));
        }
        return 0;
    }

    /**
     * Unbinds event handlers for either all of a [Creature]'s events, or one type of event.
     *
     * If `event_type` is `nil`, all the [Creature]'s event handlers are cleared.
     *
     * Otherwise, only event handlers for `event_type` are cleared.
     *
     * **NOTE:** this will affect all instances of the [Creature], not just one.
     * To bind and unbind events to a single [Creature], see [Global:RegisterUniqueCreatureEvent] and [Global:ClearUniqueCreatureEvents].
     *
     * @proto (entry)
     * @proto (entry, event_type)
     * @param uint32 entry : the ID of one or more [Creature]s whose handlers will be cleared
     * @param uint32 event_type : the event whose handlers will be cleared, see [Global:RegisterCreatureEvent]
     */
    int ClearCreatureEvents(Eluna* E)
    {
        typedef EntryKey<Hooks::CreatureEvents> Key;

        if (lua_isnoneornil(E->L, 2))
        {
            uint32 entry = Eluna::CHECKVAL<uint32>(E->L, 1);

            for (uint32 i = 1; i < Hooks::CREATURE_EVENT_COUNT; ++i)
                E->CreatureEventBindings->Clear(Key((Hooks::CreatureEvents)i, entry));
        }
        else
        {
            uint32 entry = Eluna::CHECKVAL<uint32>(E->L, 1);
            uint32 event_type = Eluna::CHECKVAL<uint32>(E->L, 2);
            E->CreatureEventBindings->Clear(Key((Hooks::CreatureEvents)event_type, entry));
        }
        return 0;
    }

    /**
     * Unbinds event handlers for either all of a [Creature]'s events, or one type of event.
     *
     * If `event_type` is `nil`, all the [Creature]'s event handlers are cleared.
     *
     * Otherwise, only event handlers for `event_type` are cleared.
     *
     * **NOTE:** this will affect only a single [Creature].
     * To bind and unbind events to all instances of a [Creature], see [Global:RegisterCreatureEvent] and [Global:ClearCreatureEvent].
     *
     * @proto (entry)
     * @proto (entry, event_type)
     * @param ObjectGuid guid : the GUID of a single [Creature] whose handlers will be cleared
     * @param uint32 instance_id : the instance ID of a single [Creature] whose handlers will be cleared
     * @param uint32 event_type : the event whose handlers will be cleared, see [Global:RegisterCreatureEvent]
     */
    int ClearUniqueCreatureEvents(Eluna* E)
    {
        typedef UniqueObjectKey<Hooks::CreatureEvents> Key;

        if (lua_isnoneornil(E->L, 3))
        {
            ObjectGuid guid = Eluna::CHECKVAL<ObjectGuid>(E->L, 1);
            uint32 instanceId = Eluna::CHECKVAL<uint32>(E->L, 2);

            for (uint32 i = 1; i < Hooks::CREATURE_EVENT_COUNT; ++i)
                E->CreatureUniqueBindings->Clear(Key((Hooks::CreatureEvents)i, guid, instanceId));
        }
        else
        {
            ObjectGuid guid = Eluna::CHECKVAL<ObjectGuid>(E->L, 1);
            uint32 instanceId = Eluna::CHECKVAL<uint32>(E->L, 2);
            uint32 event_type = Eluna::CHECKVAL<uint32>(E->L, 3);
            E->CreatureUniqueBindings->Clear(Key((Hooks::CreatureEvents)event_type, guid, instanceId));
        }
        return 0;
    }

    /**
     * Unbinds event handlers for either all of a [Creature]'s gossip events, or one type of event.
     *
     * If `event_type` is `nil`, all the [Creature]'s gossip event handlers are cleared.
     *
     * Otherwise, only event handlers for `event_type` are cleared.
     *
     * **NOTE:** this will affect all instances of the [Creature], not just one.
     * To bind and unbind gossip events to a single [Creature], tell the Eluna developers to implement that.
     *
     * @proto (entry)
     * @proto (entry, event_type)
     * @param uint32 entry : the ID of a [Creature] whose handlers will be cleared
     * @param uint32 event_type : the event whose handlers will be cleared, see [Global:RegisterCreatureGossipEvent]
     */
    int ClearCreatureGossipEvents(Eluna* E)
    {
        typedef EntryKey<Hooks::GossipEvents> Key;

        if (lua_isnoneornil(E->L, 2))
        {
            uint32 entry = Eluna::CHECKVAL<uint32>(E->L, 1);

            for (uint32 i = 1; i < Hooks::GOSSIP_EVENT_COUNT; ++i)
                E->CreatureGossipBindings->Clear(Key((Hooks::GossipEvents)i, entry));
        }
        else
        {
            uint32 entry = Eluna::CHECKVAL<uint32>(E->L, 1);
            uint32 event_type = Eluna::CHECKVAL<uint32>(E->L, 2);
            E->CreatureGossipBindings->Clear(Key((Hooks::GossipEvents)event_type, entry));
        }
        return 0;
    }

    /**
     * Unbinds event handlers for either all of a [GameObject]'s events, or one type of event.
     *
     * If `event_type` is `nil`, all the [GameObject]'s event handlers are cleared.
     *
     * Otherwise, only event handlers for `event_type` are cleared.
     *
     * **NOTE:** this will affect all instances of the [GameObject], not just one.
     * To bind and unbind events to a single [GameObject], tell the Eluna developers to implement that.
     *
     * @proto (entry)
     * @proto (entry, event_type)
     * @param uint32 entry : the ID of a [GameObject] whose handlers will be cleared
     * @param uint32 event_type : the event whose handlers will be cleared, see [Global:RegisterGameObjectEvent]
     */
    int ClearGameObjectEvents(Eluna* E)
    {
        typedef EntryKey<Hooks::GameObjectEvents> Key;

        if (lua_isnoneornil(E->L, 2))
        {
            uint32 entry = Eluna::CHECKVAL<uint32>(E->L, 1);

            for (uint32 i = 1; i < Hooks::GAMEOBJECT_EVENT_COUNT; ++i)
                E->GameObjectEventBindings->Clear(Key((Hooks::GameObjectEvents)i, entry));
        }
        else
        {
            uint32 entry = Eluna::CHECKVAL<uint32>(E->L, 1);
            uint32 event_type = Eluna::CHECKVAL<uint32>(E->L, 2);
            E->GameObjectEventBindings->Clear(Key((Hooks::GameObjectEvents)event_type, entry));
        }
        return 0;
    }

    /**
     * Unbinds event handlers for either all of a [GameObject]'s gossip events, or one type of event.
     *
     * If `event_type` is `nil`, all the [GameObject]'s gossip event handlers are cleared.
     *
     * Otherwise, only event handlers for `event_type` are cleared.
     *
     * **NOTE:** this will affect all instances of the [GameObject], not just one.
     * To bind and unbind gossip events to a single [GameObject], tell the Eluna developers to implement that.
     *
     * @proto (entry)
     * @proto (entry, event_type)
     * @param uint32 entry : the ID of a [GameObject] whose handlers will be cleared
     * @param uint32 event_type : the event whose handlers will be cleared, see [Global:RegisterGameObjectGossipEvent]
     */
    int ClearGameObjectGossipEvents(Eluna* E)
    {
        typedef EntryKey<Hooks::GossipEvents> Key;

        if (lua_isnoneornil(E->L, 2))
        {
            uint32 entry = Eluna::CHECKVAL<uint32>(E->L, 1);

            for (uint32 i = 1; i < Hooks::GOSSIP_EVENT_COUNT; ++i)
                E->GameObjectGossipBindings->Clear(Key((Hooks::GossipEvents)i, entry));
        }
        else
        {
            uint32 entry = Eluna::CHECKVAL<uint32>(E->L, 1);
            uint32 event_type = Eluna::CHECKVAL<uint32>(E->L, 2);
            E->GameObjectGossipBindings->Clear(Key((Hooks::GossipEvents)event_type, entry));
        }
        return 0;
    }

    /**
     * Unbinds event handlers for either all [Group] events, or one type of [Group] event.
     *
     * If `event_type` is `nil`, all [Group] event handlers are cleared.
     *
     * Otherwise, only event handlers for `event_type` are cleared.
     *
     * @proto ()
     * @proto (event_type)
     * @param uint32 event_type : the event whose handlers will be cleared, see [Global:RegisterGroupEvent]
     */
    int ClearGroupEvents(Eluna* E)
    {
        typedef EventKey<Hooks::GroupEvents> Key;

        if (lua_isnoneornil(E->L, 1))
        {
            E->GroupEventBindings->Clear();
        }
        else
        {
            uint32 event_type = Eluna::CHECKVAL<uint32>(E->L, 1);
            E->GroupEventBindings->Clear(Key((Hooks::GroupEvents)event_type));
        }
        return 0;
    }

    /**
     * Unbinds event handlers for either all [Guild] events, or one type of [Guild] event.
     *
     * If `event_type` is `nil`, all [Guild] event handlers are cleared.
     *
     * Otherwise, only event handlers for `event_type` are cleared.
     *
     * @proto ()
     * @proto (event_type)
     * @param uint32 event_type : the event whose handlers will be cleared, see [Global:RegisterGuildEvent]
     */
    int ClearGuildEvents(Eluna* E)
    {
        typedef EventKey<Hooks::GuildEvents> Key;

        if (lua_isnoneornil(E->L, 1))
        {
            E->GuildEventBindings->Clear();
        }
        else
        {
            uint32 event_type = Eluna::CHECKVAL<uint32>(E->L, 1);
            E->GuildEventBindings->Clear(Key((Hooks::GuildEvents)event_type));
        }
        return 0;
    }

    /**
     * Unbinds event handlers for either all of an [Item]'s events, or one type of event.
     *
     * If `event_type` is `nil`, all the [Item]'s event handlers are cleared.
     *
     * Otherwise, only event handlers for `event_type` are cleared.
     *
     * **NOTE:** this will affect all instances of the [Item], not just one.
     * To bind and unbind events to a single [Item], tell the Eluna developers to implement that.
     *
     * @proto (entry)
     * @proto (entry, event_type)
     * @param uint32 entry : the ID of an [Item] whose handlers will be cleared
     * @param uint32 event_type : the event whose handlers will be cleared, see [Global:RegisterItemEvent]
     */
    int ClearItemEvents(Eluna* E)
    {
        typedef EntryKey<Hooks::ItemEvents> Key;

        if (lua_isnoneornil(E->L, 2))
        {
            uint32 entry = Eluna::CHECKVAL<uint32>(E->L, 1);

            for (uint32 i = 1; i < Hooks::ITEM_EVENT_COUNT; ++i)
                E->ItemEventBindings->Clear(Key((Hooks::ItemEvents)i, entry));
        }
        else
        {
            uint32 entry = Eluna::CHECKVAL<uint32>(E->L, 1);
            uint32 event_type = Eluna::CHECKVAL<uint32>(E->L, 2);
            E->ItemEventBindings->Clear(Key((Hooks::ItemEvents)event_type, entry));
        }
        return 0;
    }

    /**
     * Unbinds event handlers for either all of an [Item]'s gossip events, or one type of event.
     *
     * If `event_type` is `nil`, all the [Item]'s gossip event handlers are cleared.
     *
     * Otherwise, only event handlers for `event_type` are cleared.
     *
     * **NOTE:** this will affect all instances of the [Item], not just one.
     * To bind and unbind gossip events to a single [Item], tell the Eluna developers to implement that.
     *
     * @proto (entry)
     * @proto (entry, event_type)
     * @param uint32 entry : the ID of an [Item] whose handlers will be cleared
     * @param uint32 event_type : the event whose handlers will be cleared, see [Global:RegisterItemGossipEvent]
     */
    int ClearItemGossipEvents(Eluna* E)
    {
        typedef EntryKey<Hooks::GossipEvents> Key;

        if (lua_isnoneornil(E->L, 2))
        {
            uint32 entry = Eluna::CHECKVAL<uint32>(E->L, 1);

            for (uint32 i = 1; i < Hooks::GOSSIP_EVENT_COUNT; ++i)
                E->ItemGossipBindings->Clear(Key((Hooks::GossipEvents)i, entry));
        }
        else
        {
            uint32 entry = Eluna::CHECKVAL<uint32>(E->L, 1);
            uint32 event_type = Eluna::CHECKVAL<uint32>(E->L, 2);
            E->ItemGossipBindings->Clear(Key((Hooks::GossipEvents)event_type, entry));
        }
        return 0;
    }

    /**
     * Unbinds event handlers for either all of a [WorldPacket] opcode's events, or one type of event.
     *
     * If `event_type` is `nil`, all the [WorldPacket] opcode's event handlers are cleared.
     *
     * Otherwise, only event handlers for `event_type` are cleared.
     *
     * @proto (opcode)
     * @proto (opcode, event_type)
     * @param uint32 opcode : the type of [WorldPacket] whose handlers will be cleared
     * @param uint32 event_type : the event whose handlers will be cleared, see [Global:RegisterPacketEvent]
     */
    int ClearPacketEvents(Eluna* E)
    {
        typedef EntryKey<Hooks::PacketEvents> Key;

        if (lua_isnoneornil(E->L, 2))
        {
            uint32 entry = Eluna::CHECKVAL<uint32>(E->L, 1);

            for (uint32 i = 1; i < Hooks::PACKET_EVENT_COUNT; ++i)
                E->PacketEventBindings->Clear(Key((Hooks::PacketEvents)i, entry));
        }
        else
        {
            uint32 entry = Eluna::CHECKVAL<uint32>(E->L, 1);
            uint32 event_type = Eluna::CHECKVAL<uint32>(E->L, 2);
            E->PacketEventBindings->Clear(Key((Hooks::PacketEvents)event_type, entry));
        }
        return 0;
    }

    /**
     * Unbinds event handlers for either all [Player] events, or one type of [Player] event.
     *
     * If `event_type` is `nil`, all [Player] event handlers are cleared.
     *
     * Otherwise, only event handlers for `event_type` are cleared.
     *
     * @proto ()
     * @proto (event_type)
     * @param uint32 event_type : the event whose handlers will be cleared, see [Global:RegisterPlayerEvent]
     */
    int ClearPlayerEvents(Eluna* E)
    {
        typedef EventKey<Hooks::PlayerEvents> Key;

        if (lua_isnoneornil(E->L, 1))
        {
            E->PlayerEventBindings->Clear();
        }
        else
        {
            uint32 event_type = Eluna::CHECKVAL<uint32>(E->L, 1);
            E->PlayerEventBindings->Clear(Key((Hooks::PlayerEvents)event_type));
        }
        return 0;
    }

    /**
     * Unbinds event handlers for either all of a [Player]'s gossip events, or one type of event.
     *
     * If `event_type` is `nil`, all the [Player]'s gossip event handlers are cleared.
     *
     * Otherwise, only event handlers for `event_type` are cleared.
     *
     * @proto (entry)
     * @proto (entry, event_type)
     * @param uint32 entry : the low GUID of a [Player] whose handlers will be cleared
     * @param uint32 event_type : the event whose handlers will be cleared, see [Global:RegisterPlayerGossipEvent]
     */
    int ClearPlayerGossipEvents(Eluna* E)
    {
        typedef EntryKey<Hooks::GossipEvents> Key;

        if (lua_isnoneornil(E->L, 2))
        {
            uint32 entry = Eluna::CHECKVAL<uint32>(E->L, 1);

            for (uint32 i = 1; i < Hooks::GOSSIP_EVENT_COUNT; ++i)
                E->PlayerGossipBindings->Clear(Key((Hooks::GossipEvents)i, entry));
        }
        else
        {
            uint32 entry = Eluna::CHECKVAL<uint32>(E->L, 1);
            uint32 event_type = Eluna::CHECKVAL<uint32>(E->L, 2);
            E->PlayerGossipBindings->Clear(Key((Hooks::GossipEvents)event_type, entry));
        }
        return 0;
    }

    /**
     * Unbinds event handlers for either all server events, or one type of event.
     *
     * If `event_type` is `nil`, all server event handlers are cleared.
     *
     * Otherwise, only event handlers for `event_type` are cleared.
     *
     * @proto ()
     * @proto (event_type)
     * @param uint32 event_type : the event whose handlers will be cleared, see [Global:RegisterServerEvent]
     */
    int ClearServerEvents(Eluna* E)
    {
        typedef EventKey<Hooks::ServerEvents> Key;

        if (lua_isnoneornil(E->L, 1))
        {
            E->ServerEventBindings->Clear();
        }
        else
        {
            uint32 event_type = Eluna::CHECKVAL<uint32>(E->L, 1);
            E->ServerEventBindings->Clear(Key((Hooks::ServerEvents)event_type));
        }
        return 0;
    }

    /**
     * Unbinds event handlers for either all of a non-instanced [Map]'s events, or one type of event.
     *
     * If `event_type` is `nil`, all the non-instanced [Map]'s event handlers are cleared.
     *
     * Otherwise, only event handlers for `event_type` are cleared.
     *
     * @proto (map_id)
     * @proto (map_id, event_type)
     * @param uint32 map_id : the ID of a [Map]
     * @param uint32 event_type : the event whose handlers will be cleared, see [Global:RegisterPlayerGossipEvent]
     */
    int ClearMapEvents(Eluna* E)
    {
        typedef EntryKey<Hooks::InstanceEvents> Key;

        if (lua_isnoneornil(E->L, 2))
        {
            uint32 entry = Eluna::CHECKVAL<uint32>(E->L, 1);

            for (uint32 i = 1; i < Hooks::INSTANCE_EVENT_COUNT; ++i)
                E->MapEventBindings->Clear(Key((Hooks::InstanceEvents)i, entry));
        }
        else
        {
            uint32 entry = Eluna::CHECKVAL<uint32>(E->L, 1);
            uint32 event_type = Eluna::CHECKVAL<uint32>(E->L, 2);
            E->MapEventBindings->Clear(Key((Hooks::InstanceEvents)event_type, entry));
        }

        return 0;
    }

    /**
     * Unbinds event handlers for either all of an instanced [Map]'s events, or one type of event.
     *
     * If `event_type` is `nil`, all the instanced [Map]'s event handlers are cleared.
     *
     * Otherwise, only event handlers for `event_type` are cleared.
     *
     * @proto (instance_id)
     * @proto (instance_id, event_type)
     * @param uint32 entry : the ID of an instance of a [Map]
     * @param uint32 event_type : the event whose handlers will be cleared, see [Global:RegisterInstanceEvent]
     */
    int ClearInstanceEvents(Eluna* E)
    {
        typedef EntryKey<Hooks::InstanceEvents> Key;

        if (lua_isnoneornil(E->L, 2))
        {
            uint32 entry = Eluna::CHECKVAL<uint32>(E->L, 1);

            for (uint32 i = 1; i < Hooks::INSTANCE_EVENT_COUNT; ++i)
                E->InstanceEventBindings->Clear(Key((Hooks::InstanceEvents)i, entry));
        }
        else
        {
            uint32 entry = Eluna::CHECKVAL<uint32>(E->L, 1);
            uint32 event_type = Eluna::CHECKVAL<uint32>(E->L, 2);
            E->InstanceEventBindings->Clear(Key((Hooks::InstanceEvents)event_type, entry));
        }

        return 0;
    }
    
    ElunaGlobal::ElunaRegister GlobalMethods[] =
    {
        // Hooks
        { "RegisterPacketEvent", &LuaGlobalFunctions::RegisterPacketEvent },
        { "RegisterServerEvent", &LuaGlobalFunctions::RegisterServerEvent },
        { "RegisterPlayerEvent", &LuaGlobalFunctions::RegisterPlayerEvent },
        { "RegisterGuildEvent", &LuaGlobalFunctions::RegisterGuildEvent },
        { "RegisterGroupEvent", &LuaGlobalFunctions::RegisterGroupEvent },
        { "RegisterCreatureEvent", &LuaGlobalFunctions::RegisterCreatureEvent },
        { "RegisterUniqueCreatureEvent", &LuaGlobalFunctions::RegisterUniqueCreatureEvent },
        { "RegisterCreatureGossipEvent", &LuaGlobalFunctions::RegisterCreatureGossipEvent },
        { "RegisterGameObjectEvent", &LuaGlobalFunctions::RegisterGameObjectEvent },
        { "RegisterGameObjectGossipEvent", &LuaGlobalFunctions::RegisterGameObjectGossipEvent },
        { "RegisterItemEvent", &LuaGlobalFunctions::RegisterItemEvent },
        { "RegisterItemGossipEvent", &LuaGlobalFunctions::RegisterItemGossipEvent },
        { "RegisterPlayerGossipEvent", &LuaGlobalFunctions::RegisterPlayerGossipEvent },
        { "RegisterBGEvent", &LuaGlobalFunctions::RegisterBGEvent },
        { "RegisterMapEvent", &LuaGlobalFunctions::RegisterMapEvent },
        { "RegisterInstanceEvent", &LuaGlobalFunctions::RegisterInstanceEvent },

        { "ClearBattleGroundEvents", &LuaGlobalFunctions::ClearBattleGroundEvents },
        { "ClearCreatureEvents", &LuaGlobalFunctions::ClearCreatureEvents },
        { "ClearUniqueCreatureEvents", &LuaGlobalFunctions::ClearUniqueCreatureEvents },
        { "ClearCreatureGossipEvents", &LuaGlobalFunctions::ClearCreatureGossipEvents },
        { "ClearGameObjectEvents", &LuaGlobalFunctions::ClearGameObjectEvents },
        { "ClearGameObjectGossipEvents", &LuaGlobalFunctions::ClearGameObjectGossipEvents },
        { "ClearGroupEvents", &LuaGlobalFunctions::ClearGroupEvents },
        { "ClearGuildEvents", &LuaGlobalFunctions::ClearGuildEvents },
        { "ClearItemEvents", &LuaGlobalFunctions::ClearItemEvents },
        { "ClearItemGossipEvents", &LuaGlobalFunctions::ClearItemGossipEvents },
        { "ClearPacketEvents", &LuaGlobalFunctions::ClearPacketEvents },
        { "ClearPlayerEvents", &LuaGlobalFunctions::ClearPlayerEvents },
        { "ClearPlayerGossipEvents", &LuaGlobalFunctions::ClearPlayerGossipEvents },
        { "ClearServerEvents", &LuaGlobalFunctions::ClearServerEvents },
        { "ClearMapEvents", &LuaGlobalFunctions::ClearMapEvents },
        { "ClearInstanceEvents", &LuaGlobalFunctions::ClearInstanceEvents },

        // Getters
        { "GetLuaEngine", &LuaGlobalFunctions::GetLuaEngine },
        { "GetCoreName", &LuaGlobalFunctions::GetCoreName },
        { "GetRealmID", &LuaGlobalFunctions::GetRealmID },
        { "GetCoreVersion", &LuaGlobalFunctions::GetCoreVersion },
        { "GetCoreExpansion", &LuaGlobalFunctions::GetCoreExpansion },
        { "GetStateMap", &LuaGlobalFunctions::GetStateMap, METHOD_REG_MAP },
        { "GetStateMapId", &LuaGlobalFunctions::GetStateMapId },
        { "GetStateInstanceId", &LuaGlobalFunctions::GetStateInstanceId },
        { "GetQuest", &LuaGlobalFunctions::GetQuest },
        { "GetPlayerByGUID", &LuaGlobalFunctions::GetPlayerByGUID, METHOD_REG_WORLD }, // World state method only in multistate
        { "GetPlayerByName", &LuaGlobalFunctions::GetPlayerByName, METHOD_REG_WORLD }, // World state method only in multistate
        { "GetGameTime", &LuaGlobalFunctions::GetGameTime },
        { "GetPlayersInWorld", &LuaGlobalFunctions::GetPlayersInWorld, METHOD_REG_WORLD }, // World state method only in multistate
        { "GetGuildByName", &LuaGlobalFunctions::GetGuildByName },
        { "GetGuildByLeaderGUID", &LuaGlobalFunctions::GetGuildByLeaderGUID },
        { "GetPlayerCount", &LuaGlobalFunctions::GetPlayerCount },
        { "GetPlayerGUID", &LuaGlobalFunctions::GetPlayerGUID },
        { "GetItemGUID", &LuaGlobalFunctions::GetItemGUID },
        { "GetObjectGUID", &LuaGlobalFunctions::GetObjectGUID },
        { "GetUnitGUID", &LuaGlobalFunctions::GetUnitGUID },
        { "GetGUIDLow", &LuaGlobalFunctions::GetGUIDLow },
        { "GetGUIDType", &LuaGlobalFunctions::GetGUIDType },
        { "GetGUIDEntry", &LuaGlobalFunctions::GetGUIDEntry },
        { "GetAreaName", &LuaGlobalFunctions::GetAreaName },
        { "bit_not", &LuaGlobalFunctions::bit_not },
        { "bit_xor", &LuaGlobalFunctions::bit_xor },
        { "bit_rshift", &LuaGlobalFunctions::bit_rshift },
        { "bit_lshift", &LuaGlobalFunctions::bit_lshift },
        { "bit_or", &LuaGlobalFunctions::bit_or },
        { "bit_and", &LuaGlobalFunctions::bit_and },
        { "GetItemLink", &LuaGlobalFunctions::GetItemLink },
        { "GetMapById", &LuaGlobalFunctions::GetMapById },
        { "GetCurrTime", &LuaGlobalFunctions::GetCurrTime },
        { "GetTimeDiff", &LuaGlobalFunctions::GetTimeDiff },
        { "PrintInfo", &LuaGlobalFunctions::PrintInfo },
        { "PrintError", &LuaGlobalFunctions::PrintError },
        { "PrintDebug", &LuaGlobalFunctions::PrintDebug },
        { "GetActiveGameEvents", &LuaGlobalFunctions::GetActiveGameEvents },

        // Boolean
        { "IsCompatibilityMode", &LuaGlobalFunctions::IsCompatibilityMode },
        { "IsInventoryPos", &LuaGlobalFunctions::IsInventoryPos },
        { "IsEquipmentPos", &LuaGlobalFunctions::IsEquipmentPos },
        { "IsBankPos", &LuaGlobalFunctions::IsBankPos },
        { "IsBagPos", &LuaGlobalFunctions::IsBagPos },
        { "IsGameEventActive", &LuaGlobalFunctions::IsGameEventActive },

        // Other
        { "ReloadEluna", &LuaGlobalFunctions::ReloadEluna },
        { "RunCommand", &LuaGlobalFunctions::RunCommand },
        { "SendWorldMessage", &LuaGlobalFunctions::SendWorldMessage },
        { "WorldDBQuery", &LuaGlobalFunctions::WorldDBQuery },
        { "WorldDBExecute", &LuaGlobalFunctions::WorldDBExecute },
        { "WorldDBQueryAsync", &LuaGlobalFunctions::WorldDBQueryAsync },
        { "CharDBQuery", &LuaGlobalFunctions::CharDBQuery },
        { "CharDBExecute", &LuaGlobalFunctions::CharDBExecute },
        { "CharDBQueryAsync", &LuaGlobalFunctions::CharDBQueryAsync },
        { "AuthDBQuery", &LuaGlobalFunctions::AuthDBQuery },
        { "AuthDBExecute", &LuaGlobalFunctions::AuthDBExecute },
        { "AuthDBQueryAsync", &LuaGlobalFunctions::AuthDBQueryAsync },
        { "CreateLuaEvent", &LuaGlobalFunctions::CreateLuaEvent },
        { "RemoveEventById", &LuaGlobalFunctions::RemoveEventById },
        { "RemoveEvents", &LuaGlobalFunctions::RemoveEvents },
        { "PerformIngameSpawn", &LuaGlobalFunctions::PerformIngameSpawn },
        { "CreatePacket", &LuaGlobalFunctions::CreatePacket },
        { "AddVendorItem", &LuaGlobalFunctions::AddVendorItem },
        { "VendorRemoveItem", &LuaGlobalFunctions::VendorRemoveItem },
        { "VendorRemoveAllItems", &LuaGlobalFunctions::VendorRemoveAllItems },
        { "Kick", &LuaGlobalFunctions::Kick },
        { "Ban", &LuaGlobalFunctions::Ban },
        { "SaveAllPlayers", &LuaGlobalFunctions::SaveAllPlayers },
        { "SendMail", &LuaGlobalFunctions::SendMail },
        { "AddTaxiPath", &LuaGlobalFunctions::AddTaxiPath },
        { "CreateInt64", &LuaGlobalFunctions::CreateLongLong },
        { "CreateUint64", &LuaGlobalFunctions::CreateULongLong },
        { "StartGameEvent", &LuaGlobalFunctions::StartGameEvent },
        { "StopGameEvent", &LuaGlobalFunctions::StopGameEvent },

        { NULL, NULL, METHOD_REG_NONE }
    };
}
#endif
