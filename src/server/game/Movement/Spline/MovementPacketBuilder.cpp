
#include "MovementPacketBuilder.h"
#include "MoveSpline.h"
#include "ByteBuffer.h"

namespace Movement
{
    inline void operator << (ByteBuffer& b, const Vector3& v)
    {
        b << v.x << v.y << v.z;
    }

    inline void operator >> (ByteBuffer& b, Vector3& v)
    {
        b >> v.x >> v.y >> v.z;
    }

    enum MonsterMoveType
    {
        MonsterMoveNormal       = 0,
        MonsterMoveStop         = 1,
        MonsterMoveFacingSpot   = 2,
        MonsterMoveFacingTarget = 3,
        MonsterMoveFacingAngle  = 4
    };

    void PacketBuilder::WriteCommonMonsterMovePart(const MoveSpline& move_spline, ByteBuffer& data)
    {
        MoveSplineFlag splineflags = move_spline.splineflags;

#ifdef LICH_KING
         data << uint8(0);  // sets/unsets MOVEMENTFLAG2_UNK7 (0x40)
#endif

        data << move_spline.spline.getPoint(move_spline.spline.first());
        data << move_spline.GetId();

        switch (splineflags & MoveSplineFlag::Mask_Final_Facing)
        {
            case MoveSplineFlag::Final_Target:
                data << uint8(MonsterMoveFacingTarget);
                data << move_spline.facing.target;
                break;
            case MoveSplineFlag::Final_Angle:
                data << uint8(MonsterMoveFacingAngle);
                data << move_spline.facing.angle;
                break;
            case MoveSplineFlag::Final_Point:
                data << uint8(MonsterMoveFacingSpot);
                data << move_spline.facing.f.x << move_spline.facing.f.y << move_spline.facing.f.z;
                break;
            default:
                data << uint8(MonsterMoveNormal);
                break;
        }

        // add fake Enter_Cycle flag - needed for client-side cyclic movement (client will erase first spline vertex after first cycle done)
        // also see: https://github.com/TrinityCore/TrinityCore/issues/22448
        // Apparently MoveSplineFlag::Enter_Cycle tells the client to discord the first point so that may help
        // Also SMSG_FLIGHT_SPLINE_SYNC must be used
        splineflags.enter_cycle = move_spline.isCyclic();

#ifdef LICH_KING
        data << uint32(splineflags & uint32(~MoveSplineFlag::Mask_No_Monster_Move));
#else
        // add fake Runmode flag - client has strange issues without that flag (only needed for players?)
        data << uint32((splineflags & uint32(~MoveSplineFlag::Mask_No_Monster_Move)) | MoveSplineFlag::Runmode);
#endif

#ifdef LICH_KING
        if (splineflags.animation)
        {
            data << splineflags.getAnimationId();
            data << move_spline.effect_start_time;
        }
#endif
        
        data << move_spline.Duration();

#ifdef LICH_KING
        if (splineflags.parabolic)
        {
            data << move_spline.vertical_acceleration;
            data << move_spline.effect_start_time;
        }
#endif
    }

    void PacketBuilder::WriteStopMovement(Vector3 const& pos, uint32 splineId, ByteBuffer& data)
    {
#ifdef LICH_KING
        data << uint8(0);  // sets/unsets MOVEMENTFLAG2_UNK7 (0x40)
#endif
        data << pos;
        data << splineId;
        data << uint8(MonsterMoveStop);
    }

    void WriteLinearPath(const Spline<int32>& spline, ByteBuffer& data)
    {
        uint32 last_idx = spline.getPointCount() - 3;
        const Vector3 * real_path = &spline.getPoint(1);

        data << last_idx;
        data << real_path[last_idx];   // destination
        if (last_idx > 1)
        {
            Vector3 middle = (real_path[0] + real_path[last_idx]) / 2.f;
            Vector3 offset;
            // first and last points already appended
            for (uint32 i = 1; i < last_idx; ++i)
            {
                offset = middle - real_path[i];
                data.appendPackXYZ(offset.x, offset.y, offset.z);
            }
        }
    }

    void WriteCatmullRomPath(const Spline<int32>& spline, ByteBuffer& data)
    {
        uint32 count = spline.getPointCount() - 3;
        data << count;
        data.append<Vector3>(&spline.getPoint(2), count);
    }

    void WriteCatmullRomCyclicPath(const Spline<int32>& spline, ByteBuffer& data)
    {
        uint32 count = spline.getPointCount() - 3;
        data << uint32(count + 1);
        data << spline.getPoint(1); // fake point, client will erase it from the spline after first cycle done
        data.append<Vector3>(&spline.getPoint(1), count);
    }

    void PacketBuilder::WriteMonsterMove(const MoveSpline& move_spline, ByteBuffer& data)
    {
        WriteCommonMonsterMovePart(move_spline, data);

        const Spline<int32>& spline = move_spline.spline;
        MoveSplineFlag splineflags = move_spline.splineflags;
        if (splineflags & MoveSplineFlag::Mask_CatmullRom)
        {
            if (splineflags.cyclic)
                WriteCatmullRomCyclicPath(spline, data);
            else
                WriteCatmullRomPath(spline, data);
        }
        else
            WriteLinearPath(spline, data);
    }

    void PacketBuilder::WriteCreate(const MoveSpline& move_spline, ByteBuffer& data)
    {
        //WriteClientStatus(mov, data);
        //data.append<float>(&mov.m_float_values[SpeedWalk], SpeedMaxCount);
        //if (mov.SplineEnabled())
        {
            MoveSplineFlag const& splineFlags = move_spline.splineflags;

            data << splineFlags.raw();

            if (splineFlags.final_angle)
            {
                data << move_spline.facing.angle;
            }
            else if (splineFlags.final_target)
            {
                data << move_spline.facing.target;
            }
            else if (splineFlags.final_point)
            {
                data << move_spline.facing.f.x << move_spline.facing.f.y << move_spline.facing.f.z;
            }

            data << move_spline.timePassed();
            data << move_spline.Duration();
            data << move_spline.GetId();

#ifdef LICH_KING
            data << float(1.f);                             // splineInfo.duration_mod; added in 3.1
            data << float(1.f);                             // splineInfo.duration_mod_next; added in 3.1

            data << move_spline.vertical_acceleration;      // added in 3.1
            data << move_spline.effect_start_time;          // added in 3.1
#endif

            uint32 nodes = move_spline.getPath().size();
            data << nodes;
            data.append<Vector3>(&move_spline.getPath()[0], nodes);
#ifdef LICH_KING
            data << uint8(move_spline.spline.mode());       // added in 3.1
#endif
            data << (move_spline.isCyclic() ? Vector3::zero() : move_spline.FinalDestination());
        }
    }
} // namespace Movement
