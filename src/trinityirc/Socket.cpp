/*
 * Copyright (C) 2008-2009 Trinity <http://www.trinitycore.org/>
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
 /*
 *  TrinityIRC by Machiavelli (machiaveltman@gmail.com),
 *  based on MangChat by Cybrax (cybraxvd@gmail.com).
 *  Please give proper credit for the authors' hard work.
 */

#include "Policies/SingletonImp.h"
#include "IRC.h"
#include "Message.h"
#include <openssl/ssl.h>

class IRCConnect;

#define SSL_BSIZE 80

bool IRCConnect::Connect() 
{
    /* Refer to SSL connection handler if SSL is enabled. */
    if(sIRC.Configs.Use_SSL)
        if(SSLConnect())
            return true;
        else
            return false;    

    /* Start normal socket connection. */
    
    #ifdef WIN32
    WSADATA wsaData;
    if(WSAStartup(MAKEWORD(2,0),&wsaData) != 0)
    {
        sLog.outIRC("Winsock initialization error");
        return false;
    }
    #endif

    if (IsConnected())
        return false;

    irc_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (irc_socket== -1)
    {
        sLog.outIRC("Invalid Socket");
        return false;
    }
    
    int on = 1;
    if ( setsockopt ( irc_socket, SOL_SOCKET, SO_REUSEADDR, ( const char* ) &on, sizeof ( on ) ) == -1 )
    {
        sLog.outIRC("Unable to set socket configuration settings.");
        return false;
    }
    
    #ifdef WIN32
        u_long iMode = 0;
        ioctlsocket(irc_socket, FIONBIO, &iMode);
    #else
        fcntl(irc_socket, F_SETFL, O_ASYNC);                   // set to asynchronous I/O
        int opts = fcntl ( irc_socket, F_GETFL );

        if(opts < 0)
        {
            sLog.outError("IRC: Error, non blocking failed!");
            return 0;
        }

        opts = (opts & ~O_NONBLOCK );
        fcntl(irc_socket, F_SETFL, opts);
    #endif
    
    
    he = gethostbyname(Host);
    if (!he)
    {
        #ifdef WIN32
        closesocket(irc_socket);
        #else
        close(irc_socket);
        #endif
        sLog.outIRC("Could not resolve host.");
        return false;
    }
    sout.sin_family = AF_INET;
    sout.sin_port = htons(Port);
    memcpy(&sout.sin_addr, he->h_addr, 4);

    if (connect(irc_socket, (const sockaddr*)&sout, sizeof(sout))== -1)
    {
        #ifdef WIN32
        sLog.outIRC("Failed to connect: %d", WSAGetLastError());
        closesocket(irc_socket);
        #else
        sLog.outIRC("Failed to connect to socket.");
        close(irc_socket);
        #endif
        
        return false;
    }

    SetConnected(true);

  
    Output( "PASS %s", NickServPassword);
    Output( "NICK %s", NickName);
    Output( "USER %s 0 * :%s", UserName, RealName);

    sLog.outIRC("Succesfully connected to %s:%d", Host, Port);
    return true;
 
}

bool IRCConnect::SSLConnect()
{
    int cont;
    he = gethostbyname(Host);
    memset((char*)&sout, 0, sizeof(sout));
    sout.sin_family = AF_INET;
    sout.sin_port = htons(Port);
    memcpy(&sout.sin_addr.s_addr, he->h_addr, he->h_length);

    irc_socket = socket(AF_INET, SOCK_STREAM, 0);

    SSL_library_init();
    OpenSSL_add_all_algorithms();

    method = SSLv23_client_method();
    ctx = SSL_CTX_new(method);
    ssl = SSL_new(ctx);
    cont = connect(irc_socket, (struct sockaddr *)&sout, sizeof(sout));
    if (cont == -1)
        return false;
    SSL_set_fd(ssl, irc_socket);
    SSL_connect(ssl);
   
    SetConnected(true);

    Output( "NICK %s", NickName);
    Output( "USER %s 0 * :%s", UserName, RealName);
    sLog.outIRC("Succesfully connected to %s:%d using SSL.", Host, Port);
    return true;
}

void TCClient::Decode(char *data)
{
    char* p;

    while (p = strstr(data, "\r\n"))
    {
        *p = '\0';
        HandleIncoming(data);
        data = (p+2);
    }
}

void IRCConnect::Disconnect()
{
    if( IsConnected())
    {
        sIRC.DoQuit("Leaving");
        #ifdef WIN32
            closesocket(irc_socket);
        #else
            close(irc_socket);
        #endif
        SetConnected(false);
        sLog.outIRC("Disconnected from IRC server.");
    }
}

void TCClient::HandleIncoming(char *data)
{
    sLog.outIRC(data);
    if (!strncmp(data, "PING", 4))
    {
        if (Configs.ClientPingTimeOut)
        {
            if (PingInterval == 0)
            {
                if (LastPingTime > 0)
                {
                    PingInterval = (LastPingTime - time(NULL)) + Configs.userIdleInterval; // clientIdleTime ?
                    sLog.outIRC("Ping interval of %d seconds detected.", PingInterval);
                }
            }
            LastPingTime = time(NULL);
        }
        data[1] = 'O';
        ConnectionMgr.Output(data);
        return;
    }
    
    if (!strncmp(data, "ERROR", 5))
    {
        ConnectionMgr.Disconnect();
        return;
    }
    
    if (data[0] == ':')
    {
        IRC_MSG_DATA ICD;
        char *datacpy = strdup(data);
        ICD.sender.nick = strchr(data, ':');
        if (ICD.sender.nick)
        {
            *ICD.sender.nick = '\0';
            ICD.sender.nick++;
            ICD.sender.ident = strchr(ICD.sender.nick, '!');
            if (ICD.sender.ident)
            {
                *ICD.sender.ident = '\0';
                ICD.sender.ident++;
                ICD.sender.host = strchr(ICD.sender.ident, '@');
                if (ICD.sender.host)
                {
                    *ICD.sender.host = '\0';
                    ICD.sender.host++;
                }
            }
        }
        
        ICD.command = strchr(datacpy, ' ');
        if(!ICD.command)
            return;
        *ICD.command = '\0';
        ICD.command++;

        if (strstr(ICD.command, "AUTH :***")) // some networks require us to send user data after auth request
        {
            
        }
        
        ICD.params = strchr(ICD.command, ' ');
        if (ICD.params)
        {
            *ICD.params = '\0';
            ICD.params++;
        }

        if (strlen(ICD.command) == 3)
        {   
            if (HandleRFCCode(&ICD)) 
                return;
        }

        if (HandleCTCP(ICD.sender.nick, ICD.params))
            return;
            
        GetHook(&ICD);
        free(datacpy);
    }
}

void IRCConnect::Output(const char* data, ...)
{
    va_list ap;
    char msg[513];
    va_start(ap, data);
    vsnprintf(msg, 512, data, ap );
    va_end(ap);
    
    // SSL is very specific about buffer size and content filling that size
    // SSL also requires that a linebreak is sent immediately after the text through the socket, unlike non-ssl
    bool SSL = sIRC.Configs.Use_SSL;
    if(SSL)
    {
        strcat(msg, "\r\n");
        char* msgbuf = (char *)malloc(strlen(msg)+1); 
        memset(msgbuf, 0, strlen(msg)+1);
        strncpy(msgbuf, msg, strlen(msg));
        SSL_write(ssl, msgbuf, strlen(msgbuf));
        free(msgbuf);
    }

    else
    {
        Transmit(msg);
        Transmit("\r\n");   
    } 
}

int IRCConnect::Receive()
{
    char buffer[1024];
    int ret_len;

    if (!IsConnected())
    {
        sLog.outIRC("Not connected!");
        return 1;
    }

    while (1)
    {
        ret_len = recv(irc_socket, buffer, 1023, 0);
        if(ret_len == -1 || !ret_len)
        {
            return 1;
        }
        buffer[ret_len] = '\0';
        sIRC.Decode(buffer);
    }

    return 0;

}

int IRCConnect::SSLReceive()
{
	int bytes;
    char buf[4096];

    while (1)
    {
	    bytes = SSL_read(ssl, buf, sizeof(buf));
	    if(bytes > 4096 || bytes < 1)
	    {
		    sLog.outIRC("Cannot receive data from SSL socket.");
		    return 1;
		}
    		
	    buf[bytes] = '\0';
	    sIRC.Decode(buf);
	}

	return 0;
}

void IRCConnect::Transmit(char* data)
{
    if(!IsConnected() || !irc_socket)
        return;

    if(send(irc_socket, data, (size_t)strlen(data), 0) == -1)
    {
        sLog.outIRC("Error transmitting to socket."); 
        SendAttempt++;
        ACE_Based::Thread::Sleep(3000);
    }
    
    if(SendAttempt > 20)
    {
        SendAttempt = 0;
        Disconnect();
    }
}
