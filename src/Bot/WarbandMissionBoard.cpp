/*
 * Warband Mission Board — the player-facing menu for bot raid missions.
 *
 * Two doors to the same engine:
 *   1. A gossip NPC ("Warband Mission Board", custom entry 900000, spawned at
 *      the Dalaran drop-off) listing live status plus one launch line per
 *      mission in the RAID_MISSIONS table.
 *   2. `.mission list` / `.mission start <n>` / `.mission status` chat commands.
 *
 * Launching queues the mission with RandomPlayerbotMgr; the expedition engine
 * picks it up at its next muster pass (one expedition in the field at a time —
 * a queued mission starts once the current one concludes).
 */

#include "Chat.h"
#include "Creature.h"
#include "Player.h"
#include "RandomPlayerbotMgr.h"
#include "ScriptMgr.h"
#include "ScriptedGossip.h"

using namespace Acore::ChatCommands;

namespace
{
    constexpr uint32 ACTION_STATUS = 100;

    void SendBoardFeedback(Player* player, uint8 missionIndex)
    {
        ChatHandler handler(player->GetSession());
        if (sRandomPlayerbotMgr.RequestRaidMission(missionIndex))
            handler.PSendSysMessage("Mission '{}' queued — the warband musters at the next expedition cycle.",
                                    sRandomPlayerbotMgr.GetRaidMissionName(missionIndex));
        else
            handler.PSendSysMessage("No such mission.");
    }
}

class npc_warband_mission_board : public CreatureScript
{
public:
    npc_warband_mission_board() : CreatureScript("npc_warband_mission_board") {}

    bool OnGossipHello(Player* player, Creature* creature) override
    {
        ShowBoard(player, creature);
        return true;
    }

    bool OnGossipSelect(Player* player, Creature* creature, uint32 /*sender*/, uint32 action) override
    {
        if (action == ACTION_STATUS)
        {
            ShowBoard(player, creature);  // refresh
            return true;
        }

        SendBoardFeedback(player, uint8(action));
        CloseGossipMenuFor(player);
        return true;
    }

private:
    static void ShowBoard(Player* player, Creature* creature)
    {
        ClearGossipMenuFor(player);
        AddGossipItemFor(player, GOSSIP_ICON_CHAT,
                         "Status: " + sRandomPlayerbotMgr.GetRaidMissionStatusText(),
                         GOSSIP_SENDER_MAIN, ACTION_STATUS);
        for (uint8 i = 0; i < sRandomPlayerbotMgr.GetRaidMissionCount(); ++i)
            AddGossipItemFor(player, GOSSIP_ICON_BATTLE,
                             std::string("Launch mission: ") + sRandomPlayerbotMgr.GetRaidMissionName(i),
                             GOSSIP_SENDER_MAIN, i);
        SendGossipMenuFor(player, DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());
    }
};

class WarbandMissionCommand : public CommandScript
{
public:
    WarbandMissionCommand() : CommandScript("WarbandMissionCommand") {}

    ChatCommandTable GetCommands() const override
    {
        static ChatCommandTable missionTable =
        {
            { "list",   HandleMissionListCommand,   SEC_PLAYER, Console::Yes },
            { "start",  HandleMissionStartCommand,  SEC_PLAYER, Console::Yes },
            { "status", HandleMissionStatusCommand, SEC_PLAYER, Console::Yes },
        };
        static ChatCommandTable commandTable =
        {
            { "mission", missionTable },
        };
        return commandTable;
    }

    static bool HandleMissionListCommand(ChatHandler* handler)
    {
        handler->PSendSysMessage("Warband missions:");
        for (uint8 i = 0; i < sRandomPlayerbotMgr.GetRaidMissionCount(); ++i)
            handler->PSendSysMessage("  {} — {}", i, sRandomPlayerbotMgr.GetRaidMissionName(i));
        handler->PSendSysMessage("Use .mission start <number>");
        return true;
    }

    static bool HandleMissionStartCommand(ChatHandler* handler, uint8 index)
    {
        if (sRandomPlayerbotMgr.RequestRaidMission(index))
            handler->PSendSysMessage("Mission '{}' queued — the warband musters at the next expedition cycle.",
                                     sRandomPlayerbotMgr.GetRaidMissionName(index));
        else
            handler->PSendSysMessage("No such mission. Try .mission list");
        return true;
    }

    static bool HandleMissionStatusCommand(ChatHandler* handler)
    {
        handler->PSendSysMessage("{}", sRandomPlayerbotMgr.GetRaidMissionStatusText());
        return true;
    }
};

void AddSC_warband_mission_board()
{
    new npc_warband_mission_board();
    new WarbandMissionCommand();
}
