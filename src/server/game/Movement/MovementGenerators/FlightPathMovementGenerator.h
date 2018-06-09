
#ifndef TRINITY_FLIGHTPATHMOVEMENTGENERATOR_H
#define TRINITY_FLIGHTPATHMOVEMENTGENERATOR_H

#include "DBCStructure.h"
#include "MovementGenerator.h"
#include "PathMovementBase.h"
#include <deque>

class Player;

/**
* FlightPathMovementGenerator generates movement of the player for the paths
* and hence generates ground and activities for the player.
*/
class FlightPathMovementGenerator : public MovementGeneratorMedium<Player, FlightPathMovementGenerator>, public PathMovementBase<Player, TaxiPathNodeList>
{
public:
    explicit FlightPathMovementGenerator(uint32 startNode = 0);

    MovementGeneratorType GetMovementGeneratorType() const override;
    bool GetResetPosition(Unit* owner, float& x, float& y, float& z) override;

    bool DoInitialize(Player*);
    void DoReset(Player*);
    bool DoUpdate(Player*, uint32);
    void DoDeactivate(Player*);
    void DoFinalize(Player*, bool, bool);

    TaxiPathNodeList const& GetPath() { return _path; }
    uint32 GetPathAtMapEnd() const;
    bool HasArrived() const { return _currentNode >= _path.size(); }

    void LoadPath(Player* owner);
    void SetCurrentNodeAfterTeleport();
    void SkipCurrentNode() { ++_currentNode; }
    void DoEventIfAny(Player* owner, TaxiPathNodeEntry const* node, bool departure);
    void InitEndGridInfo();
    void PreloadEndGrid();

private:
    float _endGridX; //! X coord of last node location
    float _endGridY; //! Y coord of last node location
    uint32 _endMapId; //! map Id of last node location
    uint32 _preloadTargetNode; //! node index where preloading starts

    struct TaxiNodeChangeInfo
    {
        uint32 PathIndex;
        int32 Cost;
    };
    std::deque<TaxiNodeChangeInfo> _pointsForPathSwitch; //! node indexes and costs where TaxiPath changes
};

#endif // TRINITY_FLIGHTPATHMOVEMENTGENERATOR_H
