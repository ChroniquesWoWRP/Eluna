#ifndef PLAYER_ITEM_SNAPSHOT_H
#define PLAYER_ITEM_SNAPSHOT_H

#include "ObjectGuid.h"
#include "Player.h"
#include "Item.h"

namespace Snapshots
{
    struct PlayerItemSnapshot
    {
        uint32 entry = 0;
        uint32 count = 0;
        uint32 durability = 0;
    };

    inline PlayerItemSnapshot MakeSnapshot(Item const* it, Player const* owner, uint32 countOverride = 0)
    {
        PlayerItemSnapshot s;
        if (!it) return s;

        s.entry = it->GetEntry();
        s.count = countOverride ? countOverride : it->GetCount();
        s.durability = it->GetUInt32Value(ITEM_FIELD_DURABILITY);

        return s;
    }
}

#endif // PLAYER_ITEM_SNAPSHOT_H