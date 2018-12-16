
#include "Common.h"
#include "Bag.h"
#include "ObjectMgr.h"
#include "Database/DatabaseEnv.h"
#include "Log.h"
#include "WorldPacket.h"
#include "UpdateData.h"
#include "WorldSession.h"

Bag::Bag( ): Item()
{
    m_objectType |= TYPEMASK_CONTAINER;
    m_objectTypeId = TYPEID_CONTAINER;

    m_valuesCount = CONTAINER_END;

    memset(m_bagslot, 0, sizeof(Item *) * MAX_BAG_SIZE);    // Maximum 20 Slots
}

Bag::~Bag()
{
    for(int i = 0; i < MAX_BAG_SIZE; ++i)
        if (Item *item = m_bagslot[i])
        {
            if (item->IsInWorld())
            {
                TC_LOG_ERROR("FIXME","Item %u (slot %u, bag slot %u) in bag %u (slot %u, bag slot %u, m_bagslot %u) is to be deleted but is still in world.",
                    item->GetEntry(), (uint32)item->GetSlot(), (uint32)item->GetBagSlot(),
                    GetEntry(), (uint32)GetSlot(), (uint32)GetBagSlot(), (uint32)i);
                item->RemoveFromWorld();
            }
            delete m_bagslot[i];
        }
}

void Bag::AddToWorld()
{
    Item::AddToWorld();

    for(uint32 i = 0;  i < GetBagSize(); ++i)
        if(m_bagslot[i])
            m_bagslot[i]->AddToWorld();
}

void Bag::RemoveFromWorld()
{
    for(uint32 i = 0; i < GetBagSize(); ++i)
        if(m_bagslot[i])
            m_bagslot[i]->RemoveFromWorld();

    Item::RemoveFromWorld();
}

bool Bag::Create(ObjectGuid::LowType guidlow, uint32 itemid, Player const* owner, ItemTemplate const *itemProto)
{
    if(!itemProto)
        itemProto = sObjectMgr->GetItemTemplate(itemid);

    if(!itemProto || itemProto->ContainerSlots > MAX_BAG_SIZE)
        return false;

    m_itemProto = itemProto;
    Object::_Create( guidlow, 0, HighGuid::Container );

    SetEntry(itemid);
    SetFloatValue(OBJECT_FIELD_SCALE_X, 1.0f);

    SetGuidValue(ITEM_FIELD_OWNER, owner ? owner->GetGUID() : ObjectGuid::Empty);
    SetGuidValue(ITEM_FIELD_CONTAINED, owner ? owner->GetGUID() : ObjectGuid::Empty);

    SetUInt32Value(ITEM_FIELD_MAXDURABILITY, itemProto->MaxDurability);
    SetUInt32Value(ITEM_FIELD_DURABILITY, itemProto->MaxDurability);
    SetUInt32Value(ITEM_FIELD_FLAGS, itemProto->Flags);
    SetUInt32Value(ITEM_FIELD_STACK_COUNT, 1);

    // Setting the number of Slots the Container has
    SetUInt32Value(CONTAINER_FIELD_NUM_SLOTS, itemProto->ContainerSlots);

    // Cleaning 20 slots
    for (uint8 i = 0; i < MAX_BAG_SIZE; i++)
    {
        SetUInt64Value(CONTAINER_FIELD_SLOT_1 + (i*2), 0);
        m_bagslot[i] = nullptr;
    }

    return true;
}

void Bag::SaveToDB(SQLTransaction& trans)
{
    Item::SaveToDB(trans);
}

bool Bag::LoadFromDB(ObjectGuid::LowType guid, ObjectGuid owner_guid, Field* fields, uint32 entry)
{
    if(!Item::LoadFromDB(guid, owner_guid, fields, entry))
        return false;

    SetUInt32Value(CONTAINER_FIELD_NUM_SLOTS, GetTemplate()->ContainerSlots);
    // cleanup bag content related item value fields (its will be filled correctly from `character_inventory`)
    for (int i = 0; i < MAX_BAG_SIZE; ++i)
    {
        SetUInt64Value(CONTAINER_FIELD_SLOT_1 + (i*2), 0);
        if (m_bagslot[i])
        {
            delete m_bagslot[i];
            m_bagslot[i] = nullptr;
        }
    }

    return true;
}

void Bag::DeleteFromDB(SQLTransaction& trans)
{
    for (auto& i : m_bagslot)
        if (i)
            i->DeleteFromDB(trans);

    Item::DeleteFromDB(trans);
}

uint32 Bag::GetFreeSlots() const
{
    uint32 slots = 0;
    for (uint32 i=0; i < GetBagSize(); i++)
        if (!m_bagslot[i])
            ++slots;

    return slots;
}

void Bag::RemoveItem( uint8 slot, bool /*update*/ )
{
    assert(slot < MAX_BAG_SIZE);

    if (m_bagslot[slot])
        m_bagslot[slot]->SetContainer(nullptr);

    m_bagslot[slot] = nullptr;
    SetUInt64Value(CONTAINER_FIELD_SLOT_1 + (slot * 2), 0);
}

void Bag::StoreItem( uint8 slot, Item *pItem, bool /*update*/ )
{
    if(slot > MAX_BAG_SIZE)
    {
        TC_LOG_ERROR("misc","Player GUID %u tried to manipulate packets and crash the server.", GetOwnerGUID().GetCounter());
        return;
    }

    if (pItem)
    {
        m_bagslot[slot] = pItem;
        SetUInt64Value(CONTAINER_FIELD_SLOT_1 + (slot * 2), pItem->GetGUID());
        pItem->SetGuidValue(ITEM_FIELD_CONTAINED, GetGUID());
        pItem->SetUInt64Value(ITEM_FIELD_OWNER, GetOwnerGUID());
        pItem->SetContainer(this);
        pItem->SetSlot(slot);
    }
}

void Bag::BuildCreateUpdateBlockForPlayer(UpdateData *data, Player *target) const
{
    Item::BuildCreateUpdateBlockForPlayer(data, target);

    for (uint32 i = 0; i < GetBagSize(); ++i)
        if(m_bagslot[i])
            m_bagslot[i]->BuildCreateUpdateBlockForPlayer(data, target);
}

// If the bag is empty returns true
bool Bag::IsEmpty() const
{
    for(uint32 i = 0; i < GetBagSize(); ++i)
        if (m_bagslot[i])
            return false;

    return true;
}

uint32 Bag::GetItemCount( uint32 item, Item* eItem ) const
{
    Item *pItem;
    uint32 count = 0;
    for(uint32 i=0; i < GetBagSize(); ++i)
    {
        pItem = m_bagslot[i];
        if( pItem && pItem != eItem && pItem->GetEntry() == item )
            count += pItem->GetCount();
    }

    if(eItem && eItem->GetTemplate()->GemProperties)
    {
        for(uint32 i=0; i < GetBagSize(); ++i)
        {
            pItem = m_bagslot[i];
            if( pItem && pItem != eItem && pItem->GetTemplate()->Socket[0].Color )
                count += pItem->GetGemCountWithID(item);
        }
    }

    return count;
}

uint8 Bag::GetSlotByItemGUID(ObjectGuid guid) const
{
    for(uint32 i = 0; i < GetBagSize(); ++i)
        if(m_bagslot[i] != nullptr)
            if(m_bagslot[i]->GetGUID() == guid)
                return i;

    return NULL_SLOT;
}

Item* Bag::GetItemByPos( uint8 slot ) const
{
    if( slot < GetBagSize() )
        return m_bagslot[slot];

    return nullptr;
}

std::string Bag::GetDebugInfo() const
{
    std::stringstream sstr;
    sstr << Item::GetDebugInfo();
    return sstr.str();
}
