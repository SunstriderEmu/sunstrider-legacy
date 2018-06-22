#include "ActionBasket.h"

#pragma once
namespace ai
{
class Queue
{
public:
    Queue(void) {}
public:
    ~Queue(void) = default;
public:
    void Push(std::shared_ptr<ActionBasket> action);
    void Push(std::list<std::shared_ptr<ActionBasket>> _actions);
    std::shared_ptr<ActionNode> Pop();
    std::shared_ptr<ActionBasket> Peek();
    int Size();
    void Clear();
private:
    std::list<std::shared_ptr<ActionBasket>> actions;
};
}