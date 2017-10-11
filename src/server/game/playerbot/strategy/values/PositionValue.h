#pragma once
#include "../Value.h"

namespace ai
{
    class Position
    {
    public:
        Position() : valueSet(false) {}
        void Set(double _x, double _y, double _z) { this->x = _x; this->y = _y; this->z = _z; this->valueSet = true; }
        void Reset() { valueSet = false; }
        bool isSet() { return valueSet; }

        double x, y, z;
        bool valueSet;
    };

    class PositionValue : public ManualSetValue<Position&>, public Qualified
    {
    public:
        PositionValue(PlayerbotAI* ai);

    private:
        Position position;
    };
}
