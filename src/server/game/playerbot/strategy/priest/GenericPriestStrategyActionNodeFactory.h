#pragma once

namespace ai
{
    class GenericPriestStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
    {
    public:
        GenericPriestStrategyActionNodeFactory()
        {
            creators["inner fire"] = &inner_fire;
            creators["holy nova"] = &holy_nova;
            creators["power word: fortitude"] = &power_word_fortitude;
            creators["power word: fortitude on party"] = &power_word_fortitude_on_party;
            creators["divine spirit"] = &divine_spirit;
            creators["divine spirit on party"] = &divine_spirit_on_party;
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
            creators["psychic scream"] = &psychic_scream;
            creators["fade"] = &fade;
        }
    private:
        static std::shared_ptr<ActionNode> inner_fire(PlayerbotAI* ai)
        {
            return std::make_shared<ActionNode> ("inner fire",
                /*P*/ NextAction::array({ std::make_shared<NextAction>("remove shadowform") }),
                /*A*/ ActionList(),
                /*C*/ ActionList());
        }
        static std::shared_ptr<ActionNode> holy_nova(PlayerbotAI* ai)
        {
            return std::make_shared<ActionNode> ("holy nova",
                /*P*/ NextAction::array({ std::make_shared<NextAction>("remove shadowform") }),
                /*A*/ ActionList(),
                /*C*/ ActionList());
        }
        static std::shared_ptr<ActionNode> power_word_fortitude(PlayerbotAI* ai)
        {
            return std::make_shared<ActionNode> ("power word: fortitude",
                /*P*/ NextAction::array({ std::make_shared<NextAction>("remove shadowform") }),
                /*A*/ ActionList(),
                /*C*/ ActionList());
        }
        static std::shared_ptr<ActionNode> power_word_fortitude_on_party(PlayerbotAI* ai)
        {
            return std::make_shared<ActionNode> ("power word: fortitude on party",
                /*P*/ NextAction::array({ std::make_shared<NextAction>("remove shadowform") }),
                /*A*/ ActionList(),
                /*C*/ ActionList());
        }
        static std::shared_ptr<ActionNode> divine_spirit(PlayerbotAI* ai)
        {
            return std::make_shared<ActionNode> ("divine spirit",
                /*P*/ NextAction::array({ std::make_shared<NextAction>("remove shadowform") }),
                /*A*/ ActionList(),
                /*C*/ ActionList());
        }
        static std::shared_ptr<ActionNode> divine_spirit_on_party(PlayerbotAI* ai)
        {
            return std::make_shared<ActionNode> ("divine spirit on party",
                /*P*/ NextAction::array({ std::make_shared<NextAction>("remove shadowform") }),
                /*A*/ ActionList(),
                /*C*/ ActionList());
        }
        static std::shared_ptr<ActionNode> power_word_shield(PlayerbotAI* ai)
        {
            return std::make_shared<ActionNode> ("power word: shield",
                /*P*/ NextAction::array({ std::make_shared<NextAction>("remove shadowform") }),
                /*A*/ NextAction::array({ std::make_shared<NextAction>("renew", 50.0f) }),
                /*C*/ ActionList());
        }
        static std::shared_ptr<ActionNode> power_word_shield_on_party(PlayerbotAI* ai)
        {
            return std::make_shared<ActionNode> ("power word: shield on party",
                /*P*/ NextAction::array({ std::make_shared<NextAction>("remove shadowform") }),
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
                /*P*/ NextAction::array({ std::make_shared<NextAction>("remove shadowform") }),
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
                /*A*/ NextAction::array({ std::make_shared<NextAction>("greater heal") }),
                /*C*/ ActionList());
        }
        static std::shared_ptr<ActionNode> flash_heal_on_party(PlayerbotAI* ai)
        {
            return std::make_shared<ActionNode> ("flash heal on party",
                /*P*/ NextAction::array({ std::make_shared<NextAction>("remove shadowform") }),
                /*A*/ NextAction::array({ std::make_shared<NextAction>("greater heal on party") }),
                /*C*/ ActionList());
        }
        static std::shared_ptr<ActionNode> psychic_scream(PlayerbotAI* ai)
        {
            return std::make_shared<ActionNode> ("psychic scream",
                /*P*/ ActionList(),
                /*A*/ NextAction::array({ std::make_shared<NextAction>("fade") }),
                /*C*/ ActionList());
        }
        static std::shared_ptr<ActionNode> fade(PlayerbotAI* ai)
        {
            return std::make_shared<ActionNode> ("fade",
                /*P*/ ActionList(),
                /*A*/ NextAction::array({ std::make_shared<NextAction>("flee") }),
                /*C*/ ActionList());
        }
    };
};
