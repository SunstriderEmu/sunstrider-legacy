
#ifndef __AUTHSESSION_H__
#define __AUTHSESSION_H__

#include "Common.h"
#include "ByteBuffer.h"
#include "Socket.h"
#include "BigNumber.h"
#include "QueryCallbackProcessor.h"
#include <memory>
#include <boost/asio/ip/tcp.hpp>

using boost::asio::ip::tcp;

class Field;
struct AuthHandler;

enum AuthStatus
{
	STATUS_CHALLENGE = 0,
	STATUS_LOGON_PROOF,
	STATUS_RECONNECT_PROOF,
	STATUS_AUTHED,
	STATUS_CLOSED
};

struct AccountInfo
{
    void LoadResult(Field* fields);

    uint32 Id = 0;
    std::string Login;
    bool IsLockedToIP = false;
    std::string LockCountry;
    std::string LastIP;
    uint32 FailedLogins = 0;
    bool IsBanned = false;
    bool IsPermanenetlyBanned = false;
    AccountTypes SecurityLevel = SEC_PLAYER;
    std::string TokenKey;
};

class AuthSession : public Socket<AuthSession>
{
    typedef Socket<AuthSession> AuthSocket;

public:
    static std::unordered_map<uint8, AuthHandler> InitHandlers();

    AuthSession(tcp::socket&& socket);

    void Start() override;
    bool Update() override;

    void SendPacket(ByteBuffer& packet);

protected:
    void ReadHandler() override;

private:
    bool HandleLogonChallenge();
    bool HandleLogonProof();
    bool HandleReconnectChallenge();
    bool HandleReconnectProof();
    bool HandleRealmList();

    void CheckIpCallback(PreparedQueryResult result);
    void LogonChallengeCallback(PreparedQueryResult result);
    void ReconnectChallengeCallback(PreparedQueryResult result);
    void RealmListCallback(PreparedQueryResult result);

    void SetVSFields(const std::string& rI);

    BigNumber N, s, g, v;
    BigNumber b, B;
    BigNumber K;
    BigNumber _reconnectProof;

    AuthStatus _status;
    AccountInfo _accountInfo;
    std::string _tokenKey;
    std::string _localizationName;
    std::string _os;
    std::string _ipCountry;
    uint16 _build;
    uint8 _expversion;

    QueryCallbackProcessor _queryProcessor;
};

#pragma pack(push, 1)

struct AuthHandler
{
    AuthStatus status;
    size_t packetSize;
    bool (AuthSession::*handler)();
};

#pragma pack(pop)

#endif
