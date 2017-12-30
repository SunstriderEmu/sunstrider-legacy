

#ifndef SelectableAI_h__
#define SelectableAI_h__

#include "FactoryHolder.h"

class DBPermit
{
    public:
        virtual ~DBPermit() { }
        virtual bool IsScriptNameAllowedInDB() const = 0;
};

template <class O, class AI, bool is_db_allowed = true>
struct SelectableAI : public FactoryHolder<AI, O>, public Permissible<O>, public DBPermit
{
    SelectableAI(std::string const& name) : FactoryHolder<AI, O>(name), Permissible<O>(), DBPermit() { }

    bool IsScriptNameAllowedInDB() const final override { return is_db_allowed; }
};


#endif // SelectableAI_h__