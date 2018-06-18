#pragma once
#include "Event.h"
#include "Value.h"
#include "AiObject.h"

namespace ai
{
    class NextAction;
    typedef std::vector<NextAction*> ActionList;
    class NextAction
    {
    public:

        NextAction(std::string const name, float relevance = 0.0f)
        {
            this->name = name;
            this->relevance = relevance;
        }
        NextAction(const NextAction& o)
        {
            this->name = o.name;
            this->relevance = o.relevance;
        }
        ~NextAction()
        {
        }

    public:
        std::string getName() { return name; }
        float getRelevance() {return relevance;}

    public:
        static ActionList clone(ActionList actions);
        //Merge two list (do not use input list after merging, pointers get stolen by merged list)
        static ActionList merge(ActionList what, ActionList with);
        static ActionList array(std::initializer_list<NextAction*> args);
        static void destroy(ActionList& actions);

    private:
        float relevance;
        std::string name;
    };

    //---------------------------------------------------------------------------------------------------------------------

    class ActionBasket;

    enum ActionThreatType
    {
        ACTION_THREAT_NONE = 0,
        ACTION_THREAT_SINGLE= 1,
        ACTION_THREAT_AOE = 2
    };

    class Action : public AiNamedObject
    {
    public:
        Action(PlayerbotAI* ai, std::string name = "action") : verbose(false), AiNamedObject(ai, name) { }
        virtual ~Action(void) = default;

    public:
        virtual bool Execute(Event event) { return true; }
        virtual bool isPossible() { return true; }
        virtual bool isUseful() { return true; }
        virtual ActionList getPrerequisites() { return ActionList(); }
        virtual ActionList getAlternatives() { return ActionList(); }
        virtual ActionList getContinuers() { return ActionList(); }
        virtual ActionThreatType getThreatType() { return ACTION_THREAT_NONE; }
        void Update() {}
        void Reset() {}
        virtual Unit* GetTarget();
        virtual Value<Unit*>* GetTargetValue();
        virtual std::string GetTargetName() { return "self target"; }
        void MakeVerbose() { verbose = true; }

    protected:
        bool verbose;
    };

    class ActionNode
    {
    public:
        ActionNode(std::string name, ActionList prerequisites = {}, ActionList alternatives = {}, ActionList continuers = {})
        {
            this->action = nullptr;
            this->name = name;
            this->prerequisites = prerequisites;
            this->alternatives = alternatives;
            this->continuers = continuers;
        }
        virtual ~ActionNode()
        {
            NextAction::destroy(prerequisites);
            NextAction::destroy(alternatives);
            NextAction::destroy(continuers);
        }

    public:
        Action* getAction() { return action; }
        void setAction(Action* _action) { action = _action; }
        std::string getName() { return name; }

    public:
        ActionList getContinuers() { return NextAction::merge(NextAction::clone(continuers), action->getContinuers()); }
        ActionList getAlternatives() { return NextAction::merge(NextAction::clone(alternatives), action->getAlternatives()); }
        ActionList getPrerequisites() { return NextAction::merge(NextAction::clone(prerequisites), action->getPrerequisites()); }

    private:
        std::string name;
        Action* action;
        ActionList continuers;
        ActionList alternatives;
        ActionList prerequisites;
    };

    //---------------------------------------------------------------------------------------------------------------------

    class ActionBasket
    {
    public:
        ActionBasket(ActionNode* action, float relevance, bool skipPrerequisites, Event event) :
          action(action), relevance(relevance), skipPrerequisites(skipPrerequisites), event(event) {}
        virtual ~ActionBasket(void) = default;
    public:
        float getRelevance() {return relevance;}
        ActionNode* getAction() {return action;}
        Event getEvent() { return event; }
        bool isSkipPrerequisites() { return skipPrerequisites; }
        void AmendRelevance(float k) {relevance *= k; }
        void setRelevance(float _relevance) { this->relevance = _relevance; }
    private:
        ActionNode* action;
        float relevance;
        bool skipPrerequisites;
        Event event;
    };

    //---------------------------------------------------------------------------------------------------------------------


}

#define AI_VALUE(type, name) context->GetValue<type>(name)->Get()
#define AI_VALUE2(type, name, param) context->GetValue<type>(name, param)->Get()
