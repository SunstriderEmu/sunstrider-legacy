#include "Chat.h"
#include "WorldSession.h"
#include "ReplayPlayer.h"
#include "ReplayRecorder.h"

bool ChatHandler::HandleReplayPlayCommand(const char* c)
{
    if (!c || !*c || strchr(c, '/') != NULL || strchr(c, '.') != NULL)
        return false;

    WorldSession* sess = m_session;
    if (Player* player = GetSelectedPlayer())
        sess = player->GetSession();

    std::string filename = "replays/";
    filename += c;
    bool result = sess->StartReplaying(filename);
    if (result)
        PSendSysMessage("Starting replay %s for %s", c, playerLink(sess->GetPlayerName()).c_str());
    else
        PSendSysMessage("Could not start replay %s", c);
    return true;
}

bool ChatHandler::HandleReplayForwardCommand(const char* args)
{
    int32 secsToSkip = (int32)atoi((char*)args);

    auto player = m_session->GetReplayPlayer();
    if (player == nullptr)
    {
        SendSysMessage("Not replaying currently");
        SetSentErrorMessage(true);
        return false;
    }

    player->SkipTime(secsToSkip);
    PSendSysMessage("Skipping %i ms", secsToSkip);
    return true;
}

bool ChatHandler::HandleReplaySpeedCommand(const char* args)
{
    float newRate = (float)atof((char*)args);

    auto player = m_session->GetReplayPlayer();
    if (player == nullptr)
    {
        SendSysMessage("Not replaying currently");
        SetSentErrorMessage(true);
        return false;
    }

    player->SetSpeedRate(newRate);
    PSendSysMessage("Read speed rate changed to %f", newRate);
    return true;
}


bool ChatHandler::HandleReplayStopCommand(const char* /* args */)
{
    bool result1 = m_session->StopRecording();
    if (result1)
        SendSysMessage("Stopped recording");

    bool result2 = m_session->StopReplaying();
    //if stopped, output handled in StopReplaying

    if(!result1 && !result2)
    {
        SendSysMessage("No recording to stop");
        SetSentErrorMessage(true);
        return false;
    }

    return true;
}

bool ChatHandler::HandleReplayRecordCommand(const char* args)
{
    WorldSession* sess = m_session;
    /*if (Player* player = GetSelectedPlayer())
        sess = player->GetSession();*/

    std::string recordName(args);
    if (strcmp(args, "") == 0)
        return false;

    if (sess->StartRecording(recordName))
        PSendSysMessage("Starting replay recording for %s with name %s", playerLink(sess->GetPlayerName()).c_str(), args);
    else
        PSendSysMessage("Could not start recording. (Maybe you're already recording?)");
    return true;
}
