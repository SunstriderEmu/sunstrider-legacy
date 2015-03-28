
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

        virtual SpellScript* getScript(Spell* spell) { return NULL; }

    protected:
        std::string m_name;
};

#endif
