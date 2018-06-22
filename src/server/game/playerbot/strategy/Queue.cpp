
#include "../playerbot.h"
#include "Action.h"
#include "Queue.h"

using namespace ai;


void Queue::Push(std::shared_ptr<ActionBasket> action)
{
    if (action)
    {
        for (std::list<std::shared_ptr<ActionBasket>>::iterator iter = actions.begin(); iter != actions.end(); iter++)
        {
            std::shared_ptr<ActionBasket> basket = *iter;
            if (action->getAction()->getName() == basket->getAction()->getName())
            {
                if (basket->getRelevance() < action->getRelevance())
                    basket->setRelevance(action->getRelevance());
                return;
            }
        }
        actions.push_back(action);
    }
}

void Queue::Push(std::list<std::shared_ptr<ActionBasket>> _actions)
{
    actions = _actions;
}

std::shared_ptr<ActionNode> Queue::Pop()
{
    float max = -1;
    std::shared_ptr<ActionBasket> selection = nullptr;
    for (std::list<std::shared_ptr<ActionBasket>>::iterator iter = actions.begin(); iter != actions.end(); iter++)
    {
        std::shared_ptr<ActionBasket> basket = *iter;
        if (basket->getRelevance() > max)
        {
            max = basket->getRelevance();
            selection = basket;
        }
    }
    if (selection != nullptr)
    {
        std::shared_ptr<ActionNode> action = selection->getAction();
        actions.remove(selection);
        return action;
    }
    return nullptr;
}

std::shared_ptr<ActionBasket> Queue::Peek()
{
    float max = -1;
    std::shared_ptr<ActionBasket> selection = NULL;
    for (std::list<std::shared_ptr<ActionBasket>>::iterator iter = actions.begin(); iter != actions.end(); iter++)
    {
        std::shared_ptr<ActionBasket> basket = *iter;
        if (basket->getRelevance() > max)
        {
            max = basket->getRelevance();
            selection = basket;
        }
    }
    return selection;
}

int Queue::Size()
{
    return actions.size();
}


void Queue::Clear()
{
    actions.clear();
}
