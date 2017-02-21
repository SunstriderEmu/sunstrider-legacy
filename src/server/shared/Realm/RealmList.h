
#ifndef _REALMLIST_H
#define _REALMLIST_H

#include "Common.h"
#include "Realm/Realm.h"
#include <boost/asio/ip/address.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/deadline_timer.hpp>

using namespace boost::asio;

/// Storage object for the list of realms on the server
class TC_SHARED_API RealmList
{
public:
    typedef std::map<RealmHandle, Realm> RealmMap;

    static RealmList* instance()
    {
        static RealmList instance;
        return &instance;
    }

    ~RealmList();

    void Initialize(boost::asio::io_service& ioService, uint32 updateInterval);
    void Close();

    RealmMap const& GetRealms() const { return _realms; }
    Realm const* GetRealm(RealmHandle const& id) const;

private:
    RealmList();

    void UpdateRealms(boost::system::error_code const& error);
    void UpdateRealm(RealmHandle const& id, uint32 build, const std::string& name, ip::address const& address, ip::address const& localAddr,
        ip::address const& localSubmask, uint16 port, uint8 icon, RealmFlags flag, uint8 timezone, AccountTypes allowedSecurityLevel, float population);
    
    RealmMap _realms;
    uint32   _updateInterval;
    boost::asio::deadline_timer* _updateTimer;
    boost::asio::ip::tcp::resolver* _resolver;
};

#define sRealmList RealmList::instance()
#endif
