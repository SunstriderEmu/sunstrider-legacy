
#ifndef _UPDATEFIELDSDEBUG_H
#define _UPDATEFIELDSDEBUG_H

#include "ObjectGuid.h"
#include "UpdateFields.h"
#include "WorldSession.h"
#include <iomanip>

class WorldObject;

namespace UpdateFieldsDebug
{
    enum UpdateFieldType
    {
        UPDATE_FIELD_TYPE_GUID,
        UPDATE_FIELD_TYPE_LONG,
        UPDATE_FIELD_TYPE_INT,
        UPDATE_FIELD_TYPE_BYTES,
        UPDATE_FIELD_TYPE_FLOAT,
        UPDATE_FIELD_TYPE_TWO_SHORTS,

        UPDATE_FIELD_TYPE_UNKNOWN,
    };

    /* Return base index (for multi fields index)
    For example, if you query for UNIT_FIELD_BASEATTACKTIME+1, you'll get UNIT_FIELD_BASEATTACKTIME.
    */
    TC_GAME_API Optional<int32> GetBaseIndex(TypeID type, uint32 index);

    //get field name for index. Only for units (players and creatures).
    TC_GAME_API bool GetFieldNameString(TypeID type, uint32 index, std::string& str);

    TC_GAME_API UpdateFieldType GetUpdateFieldType(TypeID type, uint32 index);

    //return field size
    TC_GAME_API uint32 InsertFieldInStream(TypeID type, uint32 index, WorldSession::SnapshotType const& values, std::stringstream& stream);

    TC_GAME_API void FillSnapshotValues(WorldObject* target, WorldSession::SnapshotType& values);
};

#endif // _UPDATEFIELDSDEBUG_H

