#pragma once

namespace ai
{
    class PriestNonCombatStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
    {
    public:
        PriestNonCombatStrategyActionNodeFactory()
        {
            creators["holy nova"] = &holy_nova;
            creators["power word: shield"] = &power_word_shield;
            creators["power word: shield on party"] = &power_word_shield_on_party;
            creators["renew"] = &renew;
            creators["renew on party"] = &renew_on_party;
            creators["greater heal"] = &greater_heal;
            creators["greater heal on party"] = &greater_heal_on_party;
            creators["heal"] = &heal;
            creators["heal on party"] = &heal_on_party;
            creators["lesser heal"] = &lesser_heal;
            creators["lesser heal on party"] = &lesser_heal_on_party;
            creators["flash heal"] = &flash_heal;
            creators["flash heal on party"] = &flash_heal_on_party;
            creators["circle of healing"] = &circle_of_healing;
        }
    private:
        static std::shared_ptr<ActionNode> holy_nova(PlayerbotAI* ai)
        {
            return std::make_shared<ActionNode> ("holy nova",
                /*P*/ NextAction::array({ std::make_shared<NextAction>("remove shadowform") }),
                /*A*/ ActionList(),
                /*C*/ ActionList());
        }
        static std::shared_ptr<ActionNode> power_word_shield(PlayerbotAI* ai)
        {
            return std::make_shared<ActionNode> ("power word: shield",
                /*P*/ ActionList(),
                /*A*/ NextAction::array({ std::make_shared<NextAction>("renew", 50.0f) }),
                /*C*/ ActionList());
        }
        static std::shared_ptr<ActionNode> power_word_shield_on_party(PlayerbotAI* ai)
        {
            return std::make_shared<ActionNode> ("power word: shield on party",
                /*P*/ ActionList(),
                /*A*/ NextAction::array({ std::make_shared<NextAction>("renew on party", 50.0f) }),
                /*C*/ ActionList());
        }
        static std::shared_ptr<ActionNode> renew(PlayerbotAI* ai)
        {
            return std::make_shared<ActionNode> ("renew",
                /*P*/ NextAction::array({ std::make_shared<NextAction>("remove shadowform") }),
                /*A*/ ActionList(),
                /*C*/ ActionList());
        }
        static std::shared_ptr<ActionNode> renew_on_party(PlayerbotAI* ai)
        {
            return std::make_shared<ActionNode> ("renew on party",
                /*P*/ NextAction::array({ std::make_shared<NextAction>("remove shadowform") }),
                /*A*/ ActionList(),
                /*C*/ ActionList());
        }
        static std::shared_ptr<ActionNode> greater_heal(PlayerbotAI* ai)
        {
            return std::make_shared<ActionNode> ("greater heal",
                /*P*/ NextAction::array({ std::make_shared<NextAction>("remove shadowform") }),
                /*A*/ NextAction::array({ std::make_shared<NextAction>("heal") }),
                /*C*/ ActionList());
        }
        static std::shared_ptr<ActionNode> greater_heal_on_party(PlayerbotAI* ai)
        {
            return std::make_shared<ActionNode> ("greater heal on party",
                /*P*/ ActionList(),
                /*A*/ NextAction::array({ std::make_shared<NextAction>("heal on party") }),
                /*C*/ ActionList());
        }
        static std::shared_ptr<ActionNode> heal(PlayerbotAI* ai)
        {
            return std::make_shared<ActionNode> ("heal",
                /*P*/ NextAction::array({ std::make_shared<NextAction>("remove shadowform") }),
                /*A*/ NextAction::array({ std::make_shared<NextAction>("lesser heal") }),
                /*C*/ ActionList());
        }
        static std::shared_ptr<ActionNode> heal_on_party(PlayerbotAI* ai)
        {
            return std::make_shared<ActionNode> ("heal on party",
                /*P*/ NextAction::array({ std::make_shared<NextAction>("remove shadowform") }),
                /*A*/ NextAction::array({ std::make_shared<NextAction>("lesser heal on party") }),
                /*C*/ ActionList());
        }
        static std::shared_ptr<ActionNode> lesser_heal(PlayerbotAI* ai)
        {
            return std::make_shared<ActionNode> ("lesser heal",
                /*P*/ NextAction::array({ std::make_shared<NextAction>("remove shadowform") }),
                /*A*/ ActionList(),
                /*C*/ ActionList());
        }
        static std::shared_ptr<ActionNode> lesser_heal_on_party(PlayerbotAI* ai)
        {
            return std::make_shared<ActionNode> ("lesser heal on party",
                /*P*/ NextAction::array({ std::make_shared<NextAction>("remove shadowform") }),
                /*A*/ ActionList(),
                /*C*/ ActionList());
        }
        static std::shared_ptr<ActionNode> flash_heal(PlayerbotAI* ai)
        {
            return std::make_shared<ActionNode> ("flash heal",
                /*P*/ NextAction::array({ std::make_shared<NextAction>("remove shadowform") }),
                /*A*/ ActionList(),
                /*C*/ ActionList());
        }
        static std::shared_ptr<ActionNode> flash_heal_on_party(PlayerbotAI* ai)
        {
            return std::make_shared<ActionNode> ("flash heal on party",
                /*P*/ NextAction::array({ std::make_shared<NextAction>("remove shadowform") }),
                /*A*/ ActionList(),
                /*C*/ ActionList());
        }
        static std::shared_ptr<ActionNode> circle_of_healing(PlayerbotAI* ai)
        {
            return std::make_shared<ActionNode> ("circle of healing",
                /*P*/ NextAction::array({ std::make_shared<NextAction>("remove shadowform") }),
                /*A*/ NextAction::array({ std::make_shared<NextAction>("flash heal on party") }),
                /*C*/ ActionList());
        }
    };

}
