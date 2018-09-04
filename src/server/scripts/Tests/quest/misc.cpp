
class QuestSharingTest : public TestCase
{
    void Test() override
    {
        uint32 const QUEST_ID = 10086;
        TestPlayer* giver = SpawnPlayer(CLASS_PRIEST, RACE_TROLL, 58);
        TestPlayer* receiver = SpawnPlayer(CLASS_PRIEST, RACE_TROLL, 58);
        GroupPlayer(giver, receiver);

        Quest const* quest = sObjectMgr->GetQuestTemplate(QUEST_ID);
        TEST_ASSERT(quest != nullptr);
        TEST_ASSERT(giver->CanAddQuest(quest, true));
        giver->AddQuest(quest, nullptr);

        WorldPacket packet(CMSG_PUSHQUESTTOPARTY, 4);
        packet << QUEST_ID;
        giver->GetSession()->HandlePushQuestToParty(packet);

        //quest details are sent to receiver... using SMSG_QUESTGIVER_QUEST_DETAILS

        packet = WorldPacket(CMSG_QUESTGIVER_ACCEPT_QUEST, 12);
        packet << giver->GetGUID();
        packet << QUEST_ID;
        receiver->GetSession()->HandleQuestgiverAcceptQuestOpcode(packet);

        TEST_ASSERT(receiver->GetQuestStatus(QUEST_ID) != QUEST_STATUS_NONE);
    }
};

void AddSC_test_quest_misc()
{
    RegisterTestCase("quest sharing", QuestSharingTest);
}
