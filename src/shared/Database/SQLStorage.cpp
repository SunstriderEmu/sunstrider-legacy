/*
 * Copyright (C) 2005-2008 MaNGOS <http://www.mangosproject.org/>
 *
 * Copyright (C) 2008 Trinity <http://www.trinitycore.org/>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "SQLStorage.h"
#include "SQLStorageImpl.h"

extern WorldDatabaseWorkerPool WorldDatabase;

const char CreatureInfosrcfmt[]="iiiiiisssiiiiiiiiiiffiffiiiiiiiiiiiffiiiiiiiiiiiiiiiiiiiiiiisiilliiisis";
const char CreatureInfodstfmt[]="iiiiiisssiiiiiiiiiiffiffiiiiiiiiiiiffiiiiiiiiiiiiiiiiiiiiiiisiilliiiiis";
const char CreatureDataAddonInfofmt[]="iiiiiiiis";
const char CreatureModelfmt[]="iffbi";
const char CreatureInfoAddonInfofmt[]="iiiiiiiis";
const char EquipmentInfofmt[]="iiiiiiiiii";
const char GameObjectInfosrcfmt[]="iiissiifiiiiiiiiiiiiiiiiiiiiiiiiss";
const char GameObjectInfodstfmt[]="iiissiifiiiiiiiiiiiiiiiiiiiiiiiisi";
const char ItemPrototypesrcfmt[]="iiiisiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiffiffiffiffiffiiiiiiiiiifiiifiiiiiifiiiiiifiiiiiifiiiiiifiiiisiiiiiiiiiiiiiiiiiiiiiiiiifsiiiii";
const char ItemPrototypedstfmt[]="iiiisiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiffiffiffiffiffiiiiiiiiiifiiifiiiiiifiiiiiifiiiiiifiiiiiifiiiisiiiiiiiiiiiiiiiiiiiiiiiiifiiiiii";
const char PageTextfmt[]="isi";
const char SpellThreatfmt[]="iif";
const char InstanceTemplatesrcfmt[]="iiiiiffffs";
const char InstanceTemplatedstfmt[]="iiiiiffffi";
const char InstanceTemplateAddonsrcfmt[]="ib";

SQLStorage sCreatureStorage(CreatureInfosrcfmt, CreatureInfodstfmt, "entry","creature_template","creature_scripts","entryorguid");
SQLStorage sCreatureDataAddonStorage(CreatureDataAddonInfofmt,"guid","creature_addon",NULL,NULL);
SQLStorage sCreatureModelStorage(CreatureModelfmt,"modelid","creature_model_info",NULL,NULL);
SQLStorage sCreatureInfoAddonStorage(CreatureInfoAddonInfofmt,"entry","creature_template_addon",NULL,NULL);
SQLStorage sEquipmentStorage(EquipmentInfofmt,"entry","creature_equip_template",NULL,NULL);
SQLStorage sGOStorage(GameObjectInfosrcfmt, GameObjectInfodstfmt, "entry","gameobject_template",NULL,NULL);
SQLStorage sItemStorage(ItemPrototypesrcfmt, ItemPrototypedstfmt, "entry","item_template",NULL,NULL);
SQLStorage sPageTextStore(PageTextfmt,"entry","page_text",NULL,NULL);
SQLStorage sSpellThreatStore(SpellThreatfmt,"entry","spell_threat",NULL,NULL);
SQLStorage sInstanceTemplate(InstanceTemplatesrcfmt, InstanceTemplatedstfmt, "map","instance_template",NULL,NULL);
SQLStorage sInstanceTemplateAddon(InstanceTemplateAddonsrcfmt, "map","instance_template_addon",NULL,NULL);

void SQLStorage::Free ()
{
    uint32 offset=0;
    for(uint32 x=0;x<iNumFields;x++)
        if (dst_format[x]==FT_STRING)
        {
            for(uint32 y=0;y<MaxEntry;y++)
                if(pIndex[y])
                    delete [] *(char**)((char*)(pIndex[y])+offset);

            offset += sizeof(char*);
        }
        else if (dst_format[x]==FT_LOGIC)
            offset += sizeof(bool);
        else if (dst_format[x]==FT_BYTE)
            offset += sizeof(char);
        else
            offset += 4;

    delete [] pIndex;
    delete [] data;
}

void SQLStorage::Load()
{
    SQLStorageLoader loader;
    loader.Load(*this);
}

