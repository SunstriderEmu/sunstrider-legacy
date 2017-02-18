#include "Chat.h"

bool ChatHandler::HandleReplayPlayCommand(const char* args)
{
	/*
    if (!c || !*c || strchr(c, '/') != NULL || strchr(c, '.') != NULL)
        return false;
    WorldSession* sess = m_session;
    if (Player* player = getSelectedPlayer())
        sess = player->GetSession();
    std::string filename = "replays/";
    filename += c;
    sess->SetReadPacket(filename.c_str());
    if (m_session->IsReplaying())
        PSendSysMessage("Starting replay %s for %s", c, playerLink(sess->GetPlayerName()).c_str());
    else
        PSendSysMessage("Could not start replay %s", c);
	*/
    return true;
}

bool ChatHandler::HandleDebugRecvPacketDumpWrite(const char* args)
{
	/*
    WorldSession* sess = m_session;
    if (Player* player = getSelectedPlayer())
        sess = player->GetSession();
    PSendSysMessage("Starting replay recording for %s", playerLink(sess->GetPlayerName()).c_str());
    sess->SetDumpRecvPackets(c);
	*/
    return true;
}

bool ChatHandler::HandleReplayForwardCommand(const char* args)
{
    // if (!m_session->IsReplaying())
    // {
        // SendSysMessage("Not replaying currently");
        // SetSentErrorMessage(true);
        // return false;
    // }
    // int32 secsToSkip = 0;
    // ExtractInt32(&c, secsToSkip);
    // m_session->ReplaySkipTime(secsToSkip);
    // PSendSysMessage("Skipping %i ms", secsToSkip);
    return true;
}

bool ChatHandler::HandleReplaySpeedCommand(const char* args)
{
    // if (!m_session->IsReplaying())
    // {
        // SendSysMessage("Not currently replaying");
        // SetSentErrorMessage(true);
        // return false;
    // }
    // float newRate = 1.0f;
    // ExtractFloat(&c, newRate);
    // m_session->SetReplaySpeedRate(newRate);
    // PSendSysMessage("Read speed rate changed to %f", newRate);
    return true;
}


bool ChatHandler::HandleReplayStopCommand(const char* args)
{
    // if (!m_session->IsReplaying())
    // {
        // SendSysMessage("Not replaying currently");
        // SetSentErrorMessage(true);
        // return false;
    // }
    // m_session->SetReadPacket(NULL);
    // SendSysMessage("Replay stopped");
    return true;
}

bool ChatHandler::HandleReplayRecordCommand(const char* args)
{
    // WorldSession* sess = m_session;
    // if (Player* player = getSelectedPlayer())
        // sess = player->GetSession();
    // PSendSysMessage("Starting replay recording for %s", playerLink(sess->GetPlayerName()).c_str());
    // sess->SetDumpPacket(c);
    return true;
}