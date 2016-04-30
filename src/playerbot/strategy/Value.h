#pragma once
#include "Action.h"
#include "Event.h"
#include "../PlayerbotAIAware.h"
#include "AiObject.h"

namespace ai
{
    class UntypedValue : public AiNamedObject
    {
    public:
        UntypedValue(PlayerbotAI* ai, std::string name) : AiNamedObject(ai, name) {}
        virtual void Update() {}
        virtual void Reset() {}
        virtual std::string Format() { return "?"; }
    };

    template<class T>
    class Value
    {
    public:
        virtual T Get() = 0;
        virtual void Set(T value) = 0;
        operator T() { return Get(); }
    };

    template<class T>
    class CalculatedValue : public UntypedValue, public Value<T>
	{
	public:
        CalculatedValue(PlayerbotAI* ai, std::string name = "value", int checkInterval = 1) : UntypedValue(ai, name),
            checkInterval(checkInterval), ticksElapsed(checkInterval)
        { }
        virtual ~CalculatedValue() {}

	public:
        virtual T Get()
        {
            if (ticksElapsed >= checkInterval) {
                ticksElapsed = 0;
                value = Calculate();
            }
            return value;
        }
        virtual void Set(T value) { this->value = value; }
        virtual void Update()
        {
            if (ticksElapsed < checkInterval) {
                ticksElapsed++;
            }
        }

    protected:
        virtual T Calculate() = 0;

    protected:
		int checkInterval;
		int ticksElapsed;
        T value;
	};

    class Uint8CalculatedValue : public CalculatedValue<uint8>
    {
    public:
        Uint8CalculatedValue(PlayerbotAI* ai, std::string name = "value", int checkInterval = 1) :
            CalculatedValue<uint8>(ai, name, checkInterval) {}

        virtual std::string Format()
        {
            std::ostringstream out; out << (int)Calculate();
            return out.str();
        }
    };

    class Uint32CalculatedValue : public CalculatedValue<uint32>
    {
    public:
        Uint32CalculatedValue(PlayerbotAI* ai, std::string name = "value", int checkInterval = 1) :
            CalculatedValue<uint32>(ai, name, checkInterval) {}

        virtual std::string Format()
        {
            std::ostringstream out; out << (int)Calculate();
            return out.str();
        }
    };

    class FloatCalculatedValue : public CalculatedValue<float>
    {
    public:
        FloatCalculatedValue(PlayerbotAI* ai, std::string name = "value", int checkInterval = 1) :
            CalculatedValue<float>(ai, name, checkInterval) {}

        virtual std::string Format()
        {
            std::ostringstream out; out << Calculate();
            return out.str();
        }
    };

    class BoolCalculatedValue : public CalculatedValue<bool>
    {
    public:
        BoolCalculatedValue(PlayerbotAI* ai, std::string name = "value", int checkInterval = 1) :
            CalculatedValue<bool>(ai, name, checkInterval) {}

        virtual std::string Format()
        {
            return Calculate() ? "true" : "false";
        }
    };

    class UnitCalculatedValue : public CalculatedValue<Unit*>
    {
    public:
        UnitCalculatedValue(PlayerbotAI* ai, std::string name = "value", int checkInterval = 1) :
            CalculatedValue<Unit*>(ai, name, checkInterval) {}

        virtual std::string Format()
        {
            Unit* unit = Calculate();
            return unit ? unit->GetName() : "<none>";
        }
    };

    class ObjectGuidListCalculatedValue : public CalculatedValue<std::list<ObjectGuid> >
    {
    public:
        ObjectGuidListCalculatedValue(PlayerbotAI* ai, std::string name = "value", int checkInterval = 1) :
            CalculatedValue<std::list<ObjectGuid> >(ai, name, checkInterval) {}

        virtual std::string Format()
        {
            std::ostringstream out; out << "{";
            std::list<ObjectGuid> guids = Calculate();
            for (std::list<ObjectGuid>::iterator i = guids.begin(); i != guids.end(); ++i)
            {
                ObjectGuid guid = *i;
                out << guid.GetRawValue() << ",";
            }
            out << "}";
            return out.str();
        }
    };

    template<class T>
    class ManualSetValue : public UntypedValue, public Value<T>
    {
    public:
        ManualSetValue(PlayerbotAI* ai, T defaultValue, std::string name = "value") :
            UntypedValue(ai, name), value(defaultValue), defaultValue(defaultValue) {}
        virtual ~ManualSetValue() {}

    public:
        virtual T Get() { return value; }
        virtual void Set(T value) { this->value = value; }
        virtual void Update() { }
        virtual void Reset() { value = defaultValue; }

    protected:
        T value;
        T defaultValue;
    };

    class UnitManualSetValue : public ManualSetValue<Unit*>
    {
    public:
        UnitManualSetValue(PlayerbotAI* ai, Unit* defaultValue, std::string name = "value") :
            ManualSetValue<Unit*>(ai, defaultValue, name) {}

        virtual std::string Format()
        {
            Unit* unit = Get();
            return unit ? unit->GetName() : "<none>";
        }
    };
}
