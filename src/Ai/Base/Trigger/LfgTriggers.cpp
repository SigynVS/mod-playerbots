/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "LfgTriggers.h"

#include "LFGMgr.h"
#include "Playerbots.h"

bool LfgProposalActiveTrigger::IsActive() { return AI_VALUE(uint32, "lfg proposal"); }

bool LfgStuckOutsideDungeonTrigger::IsActive()
{
    // Bot accepted an LFG proposal but never made it into the instance
    // (teleport failed due to combat, falling, taxi, ...). Retry while safe.
    if (!bot->IsAlive() || bot->IsInCombat() || bot->IsBeingTeleported() || bot->IsInFlight())
        return false;

    if (sLFGMgr->GetState(bot->GetGUID()) != lfg::LFG_STATE_DUNGEON)
        return false;

    Map* map = bot->GetMap();
    return map && !map->IsDungeon();
}

bool UnknownDungeonTrigger::IsActive()
{
    return botAI->HasActivePlayerMaster() && botAI->GetMaster() && botAI->GetMaster()->IsInWorld() &&
           botAI->GetMaster()->GetMap()->IsDungeon() && bot->GetMapId() == botAI->GetMaster()->GetMapId();
}
