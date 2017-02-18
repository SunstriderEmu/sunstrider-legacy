
#ifndef _UPDATEFIELDSDEBUG_H
#define _UPDATEFIELDSDEBUG_H

#include "ObjectGuid.h"
#include "UpdateFields.h"
#include <iomanip>

class UpdateFieldsDebug
{
private:
    //shouldn't be instanced
    UpdateFieldsDebug() {}
public:
    enum UpdateFieldType
    {
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
    static int32 GetBaseIndex(TypeID type, uint32 index);

    //get field name for index. Only for units (players and creatures).
    static bool GetFieldNameString(TypeID type, uint32 index, std::string& str);

    static UpdateFieldType GetUpdateFieldType(TypeID type, uint32 index);

    //return field size
    static uint32 InsertFieldInStream(TypeID type, uint32 index, std::vector<uint32> const& values, std::stringstream& stream);

};

#endif // _UPDATEFIELDSDEBUG_H

