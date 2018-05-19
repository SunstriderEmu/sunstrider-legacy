
#ifndef TRINITY_BAG_H
#define TRINITY_BAG_H

// Maximum 36 Slots ( (CONTAINER_END - CONTAINER_FIELD_SLOT_1)/2
#define MAX_BAG_SIZE 36                                     // 2.0.12

#include "ItemPrototype.h"
#include "Item.h"

class TC_GAME_API Bag : public Item
{
    public:

        Bag();
        ~Bag() override;

        void AddToWorld() override;
        void RemoveFromWorld() override;

        bool Create(ObjectGuid::LowType guidlow, uint32 itemid, Player const* owner, ItemTemplate const *itemProto) override;

        void StoreItem( uint8 slot, Item *pItem, bool update );
        void RemoveItem( uint8 slot, bool update );

        Item* GetItemByPos( uint8 slot ) const;
        uint32 GetItemCount( uint32 item, Item* eItem = nullptr ) const;

        uint8 GetSlotByItemGUID(ObjectGuid guid) const;
        bool IsEmpty() const;
        uint32 GetFreeSlots() const;
        uint32 GetBagSize() const { return GetUInt32Value(CONTAINER_FIELD_NUM_SLOTS); }

        // DB operations
        // overwrite virtual Item::SaveToDB
        void SaveToDB(SQLTransaction& trans) override;
        // overwrite virtual Item::LoadFromDB
        bool LoadFromDB(ObjectGuid::LowType guid, ObjectGuid owner_guid, Field* fields, uint32 entry) override;
        // overwrite virtual Item::DeleteFromDB
        void DeleteFromDB(SQLTransaction& trans) override;

        void BuildCreateUpdateBlockForPlayer(UpdateData *data, Player *target) const override;

    protected:

        // Bag Storage space
        Item* m_bagslot[MAX_BAG_SIZE];
};

inline Item* NewItemOrBag(ItemTemplate const * proto)
{
    return (proto->InventoryType == INVTYPE_BAG) ? new Bag : new Item;
}
#endif

