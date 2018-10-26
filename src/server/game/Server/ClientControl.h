
#ifndef __CLIENTCONTROL_H
#define __CLIENTCONTROL_H


class ClientControl
{
friend class WorldSession;

public:
    WorldSession* GetSession() { return _owner; }

    // -- Client opcodes handling
    //CMSG_SET_ACTIVE_MOVER - Client wants to activate this unit
    void ClientSetActiveMover(ObjectGuid guid);
    //CMSG_SET_NOT_ACTIVE_MOVER - Client release control of this unit
    void ClientSetNotActiveMover(MovementInfo& info);
    // --

    // -- Data from movement packet, may be used for anticheat
    void SetLastMoveClientTimestamp(uint32 timestamp) { lastMoveClientTimestamp = timestamp; }
    void SetLastMoveServerTimestamp(uint32 timestamp) { lastMoveServerTimestamp = timestamp; }
    uint32 GetLastMoveClientTimestamp() const { return lastMoveClientTimestamp; }
    uint32 GetLastMoveServerTimestamp() const { return lastMoveServerTimestamp; }
    // --

    // -- Pending changes
    typedef std::pair<uint32, PlayerMovementPendingChange> PendingChangePair;
    PendingChangePair PopPendingMovementChange();
    // Returns counter
    uint32 PushPendingMovementChange(PlayerMovementPendingChange newChange);
    bool HasPendingMovementChange() const { return !_pendingMovementChanges.empty(); }
    bool HasPendingMovementChange(uint32 counter, uint16 opcode, ObjectGuid guid, bool apply = false) const;
    bool HasPendingMovementChange(MovementChangeType changeType) const;
    void CheckPendingMovementAcks();
    //Force resolving all pending changes right now, instead of waiting for client acks
    void ResolveAllPendingChanges();
    // --

    // -- Mover 
    // if charm is set, it will tell client to switch mover to this unit
    void AllowTakeControl(Unit* target, bool charm);
    void DisallowTakeControl(Unit* target);
    void SuppressMover(Unit* target, bool apply);

    bool IsAllowedToMove(ObjectGuid guid, bool log = true);
    bool IsAllowedToTakeControl(ObjectGuid guid);
    // The unit this client is currently trying to move (may be nullptr). /!\ He may not be able to, use GetAllowedActiveMover in most cases.
    Unit* GetActiveMover() const { return _activeMover; }
    // The unit this client is currently trying to move, and is allowed to!
    Unit* GetAllowedActiveMover() const;
    void ResetActiveMover(bool onDelete = false);
    //Use only when joining a new map
    void InitActiveMover(Player* activeMover);
    // --

private:
    ClientControl(WorldSession* session); //Only WorldSession can create us
    void Update(uint32 diff);

    void SetClientControl(Unit* target, bool allowMove);

    // Timestamp on client clock of the moment the most recently processed movement packet was SENT by the client
    uint32 lastMoveClientTimestamp;
    // Timestamp on server clock of the moment the most recently processed movement packet was RECEIVED from the client
    uint32 lastMoveServerTimestamp;
    // when a player controls a unit, and when change is made to this unit which requires an ack from the client to be acted (change of speed for example), the movementCounter is incremented
    // Is this a per session counter or a per session and per unit counter? This implementation is for per session only
    std::deque<PendingChangePair> _pendingMovementChanges;
    uint32 _movementCounter;

    // describe all units this player can activate as movers. Example, a player on a vehicle has client control over himself and the vehicle at the same time.
    // Or if player is MC someone, control over himself + target player
    GuidSet _allowedClientControl;
    // Describe all units this player can directly control with move and acks packets. A client may have just activated a mover but not be allowed to control it yet.
    GuidSet _allowedClientMove;
    // Match the unit the client has designed as active Mover. /!\ Doesn't mean he actually can move it right now! Use GetAllowedActiveMover for that.
    Unit* _activeMover;
    // Spline id for mover activation process
    uint32 _pendingActiveMoverSplineId;
    // Not instant and will begin mover transfer process
    void SetActiveMover(Unit* activeMover);
    void DisallowMover(Unit* mover);
    void AllowMover(Unit* mover);
    uint32 _releaseMoverTimeout; //if reached, kick player.

    WorldSession* _owner;
};

#endif // __CLIENTCONTROL_H

