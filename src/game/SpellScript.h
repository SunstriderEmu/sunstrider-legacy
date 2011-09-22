/*
 * Copyright (C) 2009 - 2011 Windrunner
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#ifndef WR_SPELLSCRIPT_H
#define WR_SPELLSCRIPT_H

#include <string>

#include "SharedDefines.h"

class Unit;
class Aura;
class Spell;

class SpellScript
{
    public:
        SpellScript(Spell* spell) : m_spell(spell) {}

        virtual bool preventEffect(uint32 /*effIndex*/) { return false; }
        virtual void handleEffect(uint32 /*effIndex*/) {}
        virtual void handleProc() {}
        virtual void recalcDamage(Unit* /*caster*/, Unit* /*target*/, uint32& /*damage*/) {}
        virtual void onTriggered(Aura* /*triggeredBy*/) {}

    protected:
        Spell* m_spell;
};

class SpellScriptWrapper
{
    public:
        SpellScriptWrapper(std::string name) : m_name(name) {}

        std::string getName() { return m_name; }

        virtual ScriptType getScriptType() { return SCRIPT_TYPE_SPELL; }

        virtual SpellScript* getSpellScript() { return NULL; }

    protected:
        std::string m_name;
};

#endif
