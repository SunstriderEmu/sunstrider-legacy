#pragma once

#include "Action.h"
#include "Queue.h"
#include "Trigger.h"
#include "Multiplier.h"
#include "AiObjectContext.h"
#include "Strategy.h"

namespace ai
{
    class ActionExecutionListener
    {
    public:
		virtual ~ActionExecutionListener() {}

        virtual bool Before(Action* action, Event event) = 0;
        virtual bool AllowExecution(Action* action, Event event) = 0;
        virtual void After(Action* action, bool executed, Event event) = 0;
        virtual bool OverrideResult(Action* action, bool executed, Event event) = 0;
    };

    // -----------------------------------------------------------------------------------------------------------------------

    class ActionExecutionListeners : public ActionExecutionListener
    {
    public:
        virtual ~ActionExecutionListeners();

    // ActionExecutionListener
    public:
        bool Before(Action* action, Event event) override;
        bool AllowExecution(Action* action, Event event) override;
        void After(Action* action, bool executed, Event event) override;
        bool OverrideResult(Action* action, bool executed, Event event) override;

    public:
        void Add(std::shared_ptr<ActionExecutionListener> listener)
        {
            listeners.push_back(listener);
        }
        void Remove(std::shared_ptr<ActionExecutionListener> listener)
        {
            listeners.remove(listener);
        }

    private:
        std::list<std::shared_ptr<ActionExecutionListener>> listeners;
    };

    // -----------------------------------------------------------------------------------------------------------------------

    enum ActionResult
    {
        ACTION_RESULT_UNKNOWN,
        ACTION_RESULT_OK,
        ACTION_RESULT_IMPOSSIBLE,
        ACTION_RESULT_USELESS,
        ACTION_RESULT_FAILED
    };

    class Engine : public PlayerbotAIAware
    {
    public:
        Engine(PlayerbotAI* ai, AiObjectContext *factory);

        void Init();
        void addStrategy(std::string name);
        void addStrategies(std::initializer_list<std::string> args);
        bool removeStrategy(std::string name);
        bool HasStrategy(std::string name);
        void removeAllStrategies();
        void toggleStrategy(std::string name);
        std::string ListStrategies();
        bool ContainsStrategy(StrategyType type);
        void ChangeStrategy(std::string names);
        string GetLastAction() { return lastAction; }

    public:
        virtual bool DoNextAction(Unit*, int depth = 0);
        ActionResult ExecuteAction(std::string name);

    public:
        void AddActionExecutionListener(std::shared_ptr<ActionExecutionListener> listener)
        {
            actionExecutionListeners.Add(listener);
        }
        void removeActionExecutionListener(std::shared_ptr<ActionExecutionListener> listener)
        {
            actionExecutionListeners.Remove(listener);
        }

    public:
        virtual ~Engine(void);

    private:
        bool MultiplyAndPush(ActionList actions, float forceRelevance, bool skipPrerequisites, Event event);
        void Reset();
        void ProcessTriggers();
        void PushDefaultActions();
        void PushAgain(std::shared_ptr<ActionNode> actionNode, float relevance, Event event);
        std::shared_ptr<ActionNode> CreateActionNode(std::string name);
        Action* InitializeAction(ActionNode* actionNode);
        bool ListenAndExecute(Action* action, Event event);

    private:
        void LogAction(const char* format, ...);
        void LogValues();

    protected:
        Queue queue;
        std::list<std::shared_ptr<TriggerNode>> triggers;
        std::list<std::shared_ptr<Multiplier>> multipliers;
        AiObjectContext* aiObjectContext;
        std::map<string, std::shared_ptr<Strategy>> strategies;
        float lastRelevance;
        std::string lastAction;

    public:
        bool testMode;

    private:
        ActionExecutionListeners actionExecutionListeners;
    };
}
