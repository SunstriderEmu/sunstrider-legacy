
#include "../playerbot.h"

#include "Engine.h"
#include "../PlayerbotAIConfig.h"

using namespace ai;
using namespace std;

Engine::Engine(PlayerbotAI* ai, AiObjectContext *factory) : PlayerbotAIAware(ai), aiObjectContext(factory)
{
    lastRelevance = 0.0f;
    testMode = false;
}

bool ActionExecutionListeners::Before(Action* action, Event event)
{
    bool result = true;
    for (list<std::shared_ptr<ActionExecutionListener>>::iterator i = listeners.begin(); i!=listeners.end(); i++)
    {
        result &= (*i)->Before(action, event);
    }
    return result;
}

void ActionExecutionListeners::After(Action* action, bool executed, Event event)
{
    for (list<std::shared_ptr<ActionExecutionListener>>::iterator i = listeners.begin(); i!=listeners.end(); i++)
    {
        (*i)->After(action, executed, event);
    }
}

bool ActionExecutionListeners::OverrideResult(Action* action, bool executed, Event event)
{
    bool result = executed;
    for (list<std::shared_ptr<ActionExecutionListener>>::iterator i = listeners.begin(); i!=listeners.end(); i++)
    {
        result = (*i)->OverrideResult(action, result, event);
    }
    return result;
}

bool ActionExecutionListeners::AllowExecution(Action* action, Event event)
{
    bool result = true;
    for (list<std::shared_ptr<ActionExecutionListener>>::iterator i = listeners.begin(); i!=listeners.end(); i++)
    {
        result &= (*i)->AllowExecution(action, event);
    }
    return result;
}

ActionExecutionListeners::~ActionExecutionListeners()
{
    listeners.clear();
}


Engine::~Engine(void)
{
    Reset();

    strategies.clear();
}

void Engine::Reset()
{
    queue.Clear();
    triggers.clear();
    multipliers.clear();
}

void Engine::Init()
{
    Reset();

    for (auto i = strategies.begin(); i != strategies.end(); i++)
    {
        std::shared_ptr<Strategy> strategy = i->second;
        strategy->InitMultipliers(multipliers);
        strategy->InitTriggers(triggers);
        Event emptyEvent;
        MultiplyAndPush(strategy->getDefaultActions(), 0.0f, false, emptyEvent);
    }

    if (testMode)
    {
        FILE* file = fopen("test.log", "w");
        fprintf(file, "\n");
        fclose(file);
    }
}


bool Engine::DoNextAction(Unit* unit, int depth)
{
    LogAction("--- AI Tick ---");
    if (sPlayerbotAIConfig.logValuesPerTick)
        LogValues();

    bool actionExecuted = false;
    std::shared_ptr<ActionBasket> basket = NULL;

    time_t currentTime = time(0);
    aiObjectContext->Update();
    ProcessTriggers();

    int iterations = 0;
    int iterationsPerTick = queue.Size() * sPlayerbotAIConfig.iterationsPerTick;
    do {
        basket = queue.Peek();
        if (basket) {
            if (++iterations > iterationsPerTick)
                break;

            float relevance = basket->getRelevance(); // just for reference
            bool skipPrerequisites = basket->isSkipPrerequisites();
            Event event = basket->getEvent();
            // NOTE: queue.Pop() deletes basket
            std::shared_ptr<ActionNode> actionNode = queue.Pop();
            Action* action = InitializeAction(actionNode.get());

            if (!action)
            {
                LogAction("A:%s - UNKNOWN", actionNode->getName().c_str());
            }
            else if (action->isUseful())
            {
                for (list<std::shared_ptr<Multiplier>>::iterator i = multipliers.begin(); i!= multipliers.end(); i++)
                {
                    std::shared_ptr<Multiplier> multiplier = *i;
                    relevance *= multiplier->GetValue(action);
                    if (!relevance)
                    {
                        LogAction("Multiplier %s made action %s useless", multiplier->getName().c_str(), action->getName().c_str());
                        break;
                    }
                }

                if (action->isPossible() && relevance)
                {
                    if ((!skipPrerequisites || lastRelevance-relevance > 0.04) &&
                            MultiplyAndPush(actionNode->getPrerequisites(), relevance + 0.02, false, event))
                    {
                        PushAgain(actionNode, relevance + 0.01, event);
                        continue;
                    }

                    actionExecuted = ListenAndExecute(action, event);

                    if (actionExecuted)
                    {
                        LogAction("A:%s - OK", action->getName().c_str());
                        MultiplyAndPush(actionNode->getContinuers(), 0, false, event);
                        lastRelevance = relevance;
                        break;
                    }
                    else
                    {
                        MultiplyAndPush(actionNode->getAlternatives(), relevance + 0.03, false, event);
                        LogAction("A:%s - FAILED", action->getName().c_str());
                    }
                }
                else
                {
                    MultiplyAndPush(actionNode->getAlternatives(), relevance + 0.03, false, event);
                    LogAction("A:%s - IMPOSSIBLE", action->getName().c_str());
                }
            }
            else
            {
                lastRelevance = relevance;
                LogAction("A:%s - USELESS", action->getName().c_str());
            }
        }
    }
    while (basket);

    if (!basket)
    {
        lastRelevance = 0.0f;
        PushDefaultActions();
        if (queue.Peek() && depth < 2)
            return DoNextAction(unit, depth + 1);
    }

    if (time(0) - currentTime > 1) {
        LogAction("too long execution");
    }

    if (!actionExecuted)
        LogAction("no actions executed");

    return actionExecuted;
}

std::shared_ptr<ActionNode> Engine::CreateActionNode(std::string name)
{
    for (auto i = strategies.begin(); i != strategies.end(); i++)
    {
        std::shared_ptr<Strategy> strategy = i->second;
        std::shared_ptr<ActionNode> node = strategy->GetAction(name);
        if (node)
            return node;
    }
    return std::make_shared<ActionNode> (name,
        /*P*/ ActionList(),
        /*A*/ ActionList(),
        /*C*/ ActionList());
}

bool Engine::MultiplyAndPush(ActionList actions, float forceRelevance, bool skipPrerequisites, Event event)
{
    bool pushed = false;
    if (!actions.empty())
    {
        for(auto& nextAction : actions)
        {
            std::shared_ptr<ActionNode> action = CreateActionNode(nextAction->getName());
            InitializeAction(action.get());

            float k = nextAction->getRelevance();
            if (forceRelevance > 0.0f)
            {
                k = forceRelevance;
            }

            if (k > 0)
            {
                LogAction("PUSH:%s %f", action->getName().c_str(), k);
                queue.Push(std::make_shared<ActionBasket>(action, k, skipPrerequisites, event));
                pushed = true;
            }
        }
        actions.clear();
    }
    return pushed;
}

ActionResult Engine::ExecuteAction(std::string name)
{
    bool result = false;

    std::shared_ptr<ActionNode> actionNode = CreateActionNode(name);
    if (!actionNode)
        return ACTION_RESULT_UNKNOWN;

    Action* action = InitializeAction(actionNode.get());
    if (!action)
        return ACTION_RESULT_UNKNOWN;

    if (!action->isPossible())
        return ACTION_RESULT_IMPOSSIBLE;

    if (!action->isUseful())
        return ACTION_RESULT_USELESS;

    action->MakeVerbose();
    Event emptyEvent;
    result = ListenAndExecute(action, emptyEvent);
    MultiplyAndPush(action->getContinuers(), 0.0f, false, emptyEvent);
    return result ? ACTION_RESULT_OK : ACTION_RESULT_FAILED;
}

void Engine::addStrategy(std::string name)
{
    removeStrategy(name);

    std::shared_ptr<Strategy> strategy = aiObjectContext->GetStrategy(name);
    if (strategy)
    {
        set<std::string> siblings = aiObjectContext->GetSiblingStrategy(name);
        for (set<std::string>::iterator i = siblings.begin(); i != siblings.end(); i++)
            removeStrategy(*i);

        LogAction("S:+%s", strategy->getName().c_str());
        strategies[strategy->getName()] = strategy;
    }
    Init();
}

void Engine::addStrategies(std::initializer_list<std::string> args)
{
    for (auto i : args) 
        addStrategy(std::move(i));
}

bool Engine::removeStrategy(std::string name)
{
    auto i = strategies.find(name);
    if (i == strategies.end())
        return false;

    LogAction("S:-%s", name.c_str());
    strategies.erase(i);
    Init();
    return true;
}

void Engine::removeAllStrategies()
{
    strategies.clear();
    Init();
}

void Engine::toggleStrategy(std::string name)
{
    if (!removeStrategy(name))
        addStrategy(name);
}

bool Engine::HasStrategy(std::string name)
{
    return strategies.find(name) != strategies.end();
}

void Engine::ProcessTriggers()
{
    for (list<std::shared_ptr<TriggerNode>>::iterator i = triggers.begin(); i != triggers.end(); i++)
    {
        std::shared_ptr<TriggerNode> node = *i;
        if (!node)
            continue;

        std::shared_ptr<Trigger> trigger = node->getTrigger();
        if (!trigger)
        {
            trigger = aiObjectContext->GetTrigger(node->getName());
            node->setTrigger(trigger);
        }

        if (!trigger)
            continue;

        if (testMode || trigger->needCheck())
        {
            Event event = trigger->Check();
            if (!event)
                continue;

            LogAction("T:%s", trigger->getName().c_str());
            MultiplyAndPush(node->getHandlers(), 0.0f, false, event);
        }
    }
    for (list<std::shared_ptr<TriggerNode>>::iterator i = triggers.begin(); i != triggers.end(); i++)
    {
        std::shared_ptr<Trigger> trigger = (*i)->getTrigger();
        if (trigger) 
            trigger->Reset();
    }
}

void Engine::PushDefaultActions()
{
    for (auto i = strategies.begin(); i != strategies.end(); i++)
    {
        std::shared_ptr<Strategy> strategy = i->second;
        Event emptyEvent;
        MultiplyAndPush(strategy->getDefaultActions(), 0.0f, false, emptyEvent);
    }
}

string Engine::ListStrategies()
{
    std::string s = "Strategies: ";

    if (strategies.empty())
        return s;

    for (auto i = strategies.begin(); i != strategies.end(); i++)
    {
        s.append(i->first);
        s.append(", ");
    }
    return s.substr(0, s.length() - 2);
}

void Engine::PushAgain(std::shared_ptr<ActionNode> actionNode, float relevance, Event event)
{
    ActionList nextActions(1);
    nextActions[0] = std::make_shared<NextAction>(actionNode->getName(), relevance);
    MultiplyAndPush(nextActions, relevance, true, event);
}

bool Engine::ContainsStrategy(StrategyType type)
{
    for (auto i = strategies.begin(); i != strategies.end(); i++)
    {
        std::shared_ptr<Strategy>& strategy = i->second;
        if (strategy->GetType() & type)
            return true;
    }
    return false;
}

Action* Engine::InitializeAction(ActionNode* actionNode)
{
    std::shared_ptr<Action>& action = actionNode->getAction();
    if (!action)
    {
        action = aiObjectContext->GetAction(actionNode->getName());
        actionNode->setAction(action);
    }
    return action.get();
}

bool Engine::ListenAndExecute(Action* action, Event event)
{
    bool actionExecuted = false;

    if (actionExecutionListeners.Before(action, event))
    {
        actionExecuted = actionExecutionListeners.AllowExecution(action, event) ? action->Execute(event) : true;
    }

    actionExecuted = actionExecutionListeners.OverrideResult(action, actionExecuted, event);
    actionExecutionListeners.After(action, actionExecuted, event);
    return actionExecuted;
}

void Engine::LogAction(const char* format, ...)
{
    char buf[1024];

    va_list ap;
    va_start(ap, format);
    vsprintf(buf, format, ap);
    va_end(ap);
    lastAction += "|";
    lastAction += buf;
    if (lastAction.size() > 512)
    {
        lastAction = lastAction.substr(512);
        size_t pos = lastAction.find("|");
        lastAction = (pos == string::npos ? "" : lastAction.substr(pos));
    }

    if (testMode)
    {
        FILE* file = fopen("test.log", "a");
        fprintf(file, "%s", buf);
        fprintf(file, "\n");
        fclose(file);
    }
    else
    {
        Player* bot = ai->GetBot();
        if (sPlayerbotAIConfig.logInGroupOnly && !bot->GetGroup())
            return;

        sLog->outMessage("playerbot", LOG_LEVEL_DEBUG, "%s %s", bot->GetName().c_str(), buf);
    }
}

void Engine::ChangeStrategy(std::string names)
{
    vector<std::string> splitted = split(names, ',');
    for (vector<std::string>::iterator i = splitted.begin(); i != splitted.end(); i++)
    {
        const char* name = i->c_str();
        switch (name[0])
        {
        case '+':
            addStrategy(name+1);
            break;
        case '-':
            removeStrategy(name+1);
            break;
        case '~':
            toggleStrategy(name+1);
            break;
        case '?':
            ai->TellMaster(ListStrategies());
            break;
        }
    }
}

void Engine::LogValues()
{
    if (testMode)
        return;

    Player* bot = ai->GetBot();
    if (sPlayerbotAIConfig.logInGroupOnly && !bot->GetGroup())
        return;

    std::string text = ai->GetAiObjectContext()->FormatValues();
    sLog->outMessage("playerbot", LOG_LEVEL_DEBUG, "Values for %s: %s", bot->GetName().c_str(), text.c_str());
}
