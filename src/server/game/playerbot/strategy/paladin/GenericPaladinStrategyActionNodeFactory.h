#pragma once

namespace ai
{
    class GenericPaladinStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
    {
    public:
        GenericPaladinStrategyActionNodeFactory()
        {
            creators["seal of light"] = &seal_of_light;
            creators["cleanse poison"] = &cleanse_poison;
            creators["cleanse disease"] = &cleanse_disease;
            creators["cleanse magic"] = &cleanse_magic;
            creators["cleanse poison on party"] = &cleanse_poison_on_party;
            creators["cleanse disease on party"] = &cleanse_disease_on_party;
            creators["seal of wisdom"] = &seal_of_wisdom;
            creators["seal of justice"] = &seal_of_justice;
            creators["hand of reckoning"] = &hand_of_reckoning;
            creators["judgement of wisdom"] = &judgement_of_wisdom;
            creators["divine shield"] = &divine_shield;
            creators["flash of light"] = &flash_of_light;
            creators["flash of light on party"] = &flash_of_light_on_party;
            creators["holy wrath"] = &holy_wrath;
            creators["lay on hands"] = &lay_on_hands;
            creators["lay on hands on party"] = &lay_on_hands_on_party;
        }
    private:
        static std::shared_ptr<ActionNode> lay_on_hands(PlayerbotAI* ai)
        {
            return std::make_shared<ActionNode> ("lay on hands",
                /*P*/ ActionList(),
                /*A*/ NextAction::array({ std::make_shared<NextAction>("divine shield"), std::make_shared<NextAction>("flash of light") }),
                /*C*/ ActionList());
        }
        static std::shared_ptr<ActionNode> lay_on_hands_on_party(PlayerbotAI* ai)
        {
            return std::make_shared<ActionNode> ("lay on hands on party",
                /*P*/ ActionList(),
                /*A*/ NextAction::array({ std::make_shared<NextAction>("flash of light") }),
                /*C*/ ActionList());
        }
        static std::shared_ptr<ActionNode> seal_of_light(PlayerbotAI* ai)
        {
            return std::make_shared<ActionNode> ("seal of light",
                /*P*/ ActionList(),
                /*A*/ NextAction::array({ std::make_shared<NextAction>("seal of justice") }),
                /*C*/ ActionList());
        }
        static std::shared_ptr<ActionNode> cleanse_poison(PlayerbotAI* ai)
        {
            return std::make_shared<ActionNode> ("cleanse poison",
                /*P*/ ActionList(),
                /*A*/ NextAction::array({ std::make_shared<NextAction>("purify poison") }),
                /*C*/ ActionList());
        }
        static std::shared_ptr<ActionNode> cleanse_magic(PlayerbotAI* ai)
        {
            return std::make_shared<ActionNode> ("cleanse magic",
                /*P*/ ActionList(),
                /*A*/ ActionList(),
                /*C*/ ActionList());
        }
        static std::shared_ptr<ActionNode> cleanse_disease(PlayerbotAI* ai)
        {
            return std::make_shared<ActionNode> ("cleanse disease",
                /*P*/ ActionList(),
                /*A*/ NextAction::array({ std::make_shared<NextAction>("purify disease") }),
                /*C*/ ActionList());
        }
        static std::shared_ptr<ActionNode> cleanse_poison_on_party(PlayerbotAI* ai)
        {
            return std::make_shared<ActionNode> ("cleanse poison on party",
                /*P*/ ActionList(),
                /*A*/ NextAction::array({ std::make_shared<NextAction>("purify poison on party") }),
                /*C*/ ActionList());
        }
        static std::shared_ptr<ActionNode> cleanse_disease_on_party(PlayerbotAI* ai)
        {
            return std::make_shared<ActionNode> ("cleanse disease on party",
                /*P*/ ActionList(),
                /*A*/ NextAction::array({ std::make_shared<NextAction>("purify disease on party") }),
                /*C*/ ActionList());
        }
        static std::shared_ptr<ActionNode> seal_of_wisdom(PlayerbotAI* ai)
        {
            return std::make_shared<ActionNode> ("seal of wisdom",
                /*P*/ ActionList(),
                /*A*/ NextAction::array({ std::make_shared<NextAction>("seal of justice") }),
                /*C*/ ActionList());
        }
        static std::shared_ptr<ActionNode> seal_of_justice(PlayerbotAI* ai)
        {
            return std::make_shared<ActionNode> ("seal of justice",
                /*P*/ ActionList(),
                /*A*/ NextAction::array({ std::make_shared<NextAction>("seal of righteousness") }),
                /*C*/ ActionList());
        }
        static std::shared_ptr<ActionNode> hand_of_reckoning(PlayerbotAI* ai)
        {
            return std::make_shared<ActionNode> ("hand of reckoning",
                /*P*/ ActionList(),
                /*A*/ NextAction::array({ std::make_shared<NextAction>("judgement of justice") }),
                /*C*/ ActionList());
        }
        static std::shared_ptr<ActionNode> judgement_of_wisdom(PlayerbotAI* ai)
        {
            return std::make_shared<ActionNode> ("judgement of wisdom",
                /*P*/ ActionList(),
                /*A*/ NextAction::array({ std::make_shared<NextAction>("judgement of light") }),
                /*C*/ ActionList());
        }
        static std::shared_ptr<ActionNode> divine_shield(PlayerbotAI* ai)
        {
            return std::make_shared<ActionNode> ("divine shield",
                /*P*/ ActionList(),
                /*A*/ NextAction::array({ std::make_shared<NextAction>("divine protection") }),
                /*C*/ ActionList());
        }
        static std::shared_ptr<ActionNode> flash_of_light(PlayerbotAI* ai)
        {
            return std::make_shared<ActionNode> ("flash of light",
                /*P*/ ActionList(),
                /*A*/ NextAction::array({ std::make_shared<NextAction>("holy light") }),
                /*C*/ ActionList());
        }
        static std::shared_ptr<ActionNode> flash_of_light_on_party(PlayerbotAI* ai)
        {
            return std::make_shared<ActionNode> ("flash of light on party",
                /*P*/ ActionList(),
                /*A*/ NextAction::array({ std::make_shared<NextAction>("holy light on party") }),
                /*C*/ ActionList());
        }
        static std::shared_ptr<ActionNode> holy_wrath(PlayerbotAI* ai)
        {
            return std::make_shared<ActionNode> ("holy wrath",
                /*P*/ ActionList(),
                /*A*/ NextAction::array({ std::make_shared<NextAction>("consecration") }),
                /*C*/ ActionList());
        }
    };

};
