/*
 * Copyright (C) 2008-2014 TrinityCore <http://www.trinitycore.org/>
 * Copyright (C) 2005-2009 MaNGOS <http://getmangos.com/>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _AUTHCRYPT_H
#define _AUTHCRYPT_H

#include "Cryptography/ARC4.h"

class BigNumber;
enum ClientBuild : uint32;

#ifdef LICH_KING
class AuthCrypt
{
    public:
        AuthCrypt();

        void Init(BigNumber* K);
        void DecryptRecv(uint8 *, size_t);
        void EncryptSend(uint8 *, size_t);

        bool IsInitialized() const { return _initialized; }

    private:
        ARC4 _clientDecrypt;
        ARC4 _serverEncrypt;
        bool _initialized;
};
#else
#include <vector>
class AuthCrypt
{
    public:
        AuthCrypt(ClientBuild build);

        const static size_t CRYPTED_SEND_LEN = 4;
        const static size_t CRYPTED_RECV_LEN = 6;

        void Init(BigNumber *);

        void DecryptRecv(uint8 *, size_t);
        void EncryptSend(uint8 *, size_t);

        bool IsInitialized() { return _initialized; }


    private:
        bool _initialized;
        ClientBuild clientBuild;

        //only used for LK
        ARC4* _clientDecrypt;
        ARC4* _serverEncrypt;

        //only used for BC
        std::vector<uint8> _key;
        uint8 _send_i, _send_j, _recv_i, _recv_j;
};
#endif
#endif
