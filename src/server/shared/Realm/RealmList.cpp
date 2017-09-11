
#include <boost/asio/ip/tcp.hpp>
#include "Common.h"
#include "RealmList.h"
#include "Database/DatabaseEnv.h"

namespace boost { namespace asio { namespace ip { class address; } } }

RealmList::RealmList() : _updateInterval(0), _updateTimer(nullptr), _resolver(nullptr) { }
RealmList::~RealmList()
{
    delete _resolver;
    delete _updateTimer;
}

// Load the realm list from the database
void RealmList::Initialize(boost::asio::io_service& ioService, uint32 updateInterval)
{
    _updateInterval = updateInterval;
    _updateTimer = new boost::asio::deadline_timer(ioService);
    _resolver = new boost::asio::ip::tcp::resolver(ioService);
    
    // Get the content of the realmlist table in the database
    UpdateRealms(boost::system::error_code());
}

void RealmList::Close()
{
    _updateTimer->cancel();
}

void RealmList::UpdateRealm(RealmHandle const& id, uint32 build, const std::string& name, ip::address const& address, ip::address const& localAddr,
    ip::address const& localSubmask, uint16 port, uint8 icon, RealmFlags flag, uint8 timezone, AccountTypes allowedSecurityLevel,
    float population)
{
    // Create new if not exist or update existed
    Realm& realm = _realms[id];

    realm.Id = id;
    realm.Build = build;
    realm.Name = name;
    realm.Type = icon;
    realm.Flags = flag;
    realm.Timezone = timezone;
    realm.AllowedSecurityLevel = allowedSecurityLevel;
    realm.PopulationLevel = population;
    realm.ExternalAddress = address;
    realm.LocalAddress = localAddr;
    realm.LocalSubnetMask = localSubmask;
    realm.Port = port;
}

void RealmList::UpdateRealms(boost::system::error_code const& error)
{
    if (error)
        return;

    TC_LOG_INFO("server.authserver", "Updating Realm List...");

    PreparedStatement* stmt = LoginDatabase.GetPreparedStatement(LOGIN_SEL_REALMLIST);
    PreparedQueryResult result = LoginDatabase.Query(stmt);

    std::map<RealmHandle, std::string> existingRealms;
    for (auto const& p : _realms)
        existingRealms[p.first] = p.second.Name;

    _realms.clear();

    // Circle through results and add them to the realm map
    if (result)
    {
        do
        {
            try
            {
                boost::asio::ip::tcp::resolver::iterator end;

                Field* fields = result->Fetch();
                uint32 realmId = fields[0].GetUInt32();
                std::string name = fields[1].GetString();
                boost::asio::ip::tcp::resolver::query externalAddressQuery(ip::tcp::v4(), fields[2].GetString(), "");

                boost::system::error_code ec;
                boost::asio::ip::tcp::resolver::iterator endPoint = _resolver->resolve(externalAddressQuery, ec);
                if (endPoint == end || ec)
                {
                    TC_LOG_ERROR("server.authserver", "Could not resolve address %s", fields[2].GetString().c_str());
                    return;
                }

                ip::address externalAddress = (*endPoint).endpoint().address();

                boost::asio::ip::tcp::resolver::query localAddressQuery(ip::tcp::v4(), fields[3].GetString(), "");
                endPoint = _resolver->resolve(localAddressQuery, ec);
                if (endPoint == end || ec)
                {
                    TC_LOG_ERROR("server.authserver", "Could not resolve address %s", fields[3].GetString().c_str());
                    return;
                }

                ip::address localAddress = (*endPoint).endpoint().address();

                boost::asio::ip::tcp::resolver::query localSubmaskQuery(ip::tcp::v4(), fields[4].GetString(), "");
                endPoint = _resolver->resolve(localSubmaskQuery, ec);
                if (endPoint == end || ec)
                {
                    TC_LOG_ERROR("server.authserver", "Could not resolve address %s", fields[4].GetString().c_str());
                    return;
                }

                ip::address localSubmask = (*endPoint).endpoint().address();

                uint16 port = fields[5].GetUInt16();
                uint8 icon = fields[6].GetUInt8();
                if (icon == REALM_TYPE_FFA_PVP)
                    icon = REALM_TYPE_PVP;
                if (icon >= MAX_CLIENT_REALM_TYPE)
                    icon = REALM_TYPE_NORMAL;
                RealmFlags flag = RealmFlags(fields[7].GetUInt8());
                uint8 timezone = fields[8].GetUInt8();
                uint8 allowedSecurityLevel = fields[9].GetUInt8();
                float pop = fields[10].GetFloat();
                uint32 build = fields[11].GetUInt32();

                RealmHandle id{ realmId };

                UpdateRealm(id, build, name, externalAddress, localAddress, localSubmask, port, icon, flag,
                    timezone, (allowedSecurityLevel <= SEC_ADMINISTRATOR ? AccountTypes(allowedSecurityLevel) : SEC_ADMINISTRATOR), pop);

                if (!existingRealms.count(id))
                    TC_LOG_INFO("server.authserver", "Added realm \"%s\" at %s:%u.", name.c_str(), externalAddress.to_string().c_str(), port);
                else
                    TC_LOG_DEBUG("server.authserver", "Updating realm \"%s\" at %s:%u.", name.c_str(), externalAddress.to_string().c_str(), port);

                existingRealms.erase(id);
            }
            catch (std::exception& ex)
            {
                TC_LOG_ERROR("server.authserver", "Realmlist::UpdateRealms has thrown an exception: %s", ex.what());
                ABORT();
            }
        }
        while (result->NextRow());
    }

    for (auto itr = existingRealms.begin(); itr != existingRealms.end(); ++itr)
        TC_LOG_INFO("server.authserver", "Removed realm \"%s\".", itr->second.c_str());

    if (_updateInterval)
    {
        _updateTimer->expires_from_now(boost::posix_time::seconds(_updateInterval));
        _updateTimer->async_wait(std::bind(&RealmList::UpdateRealms, this, std::placeholders::_1));
    }
}

Realm const* RealmList::GetRealm(RealmHandle const& id) const
{
    auto itr = _realms.find(id);
    if (itr != _realms.end())
        return &itr->second;

    return nullptr;
}