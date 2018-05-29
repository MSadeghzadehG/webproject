



#pragma once

#include <memory>
#include <libdevcrypto/Common.h>
#include "RLPXSocket.h"
#include "RLPXFrameCoder.h"
#include "Common.h"
namespace ba = boost::asio;
namespace bi = boost::asio::ip;

namespace dev
{
namespace p2p
{

static const unsigned c_rlpxVersion = 4;


class RLPXHandshake: public std::enable_shared_from_this<RLPXHandshake>
{
    friend class RLPXFrameCoder;

public:
        RLPXHandshake(Host* _host, std::shared_ptr<RLPXSocket> const& _socket): m_host(_host), m_originated(false), m_socket(_socket), m_idleTimer(m_socket->ref().get_io_service()) { crypto::Nonce::get().ref().copyTo(m_nonce.ref()); }
    
        RLPXHandshake(Host* _host, std::shared_ptr<RLPXSocket> const& _socket, NodeID _remote): m_host(_host), m_remote(_remote), m_originated(true), m_socket(_socket), m_idleTimer(m_socket->ref().get_io_service()) { crypto::Nonce::get().ref().copyTo(m_nonce.ref()); }

    virtual ~RLPXHandshake() = default;

        void start() { transition(); }

        void cancel();

protected:
        enum State
    {
        Error = -1,
        New,
        AckAuth,
        AckAuthEIP8,
        WriteHello,
        ReadHello,
        StartSession
    };

        void writeAuth();

        void readAuth();

        void readAuthEIP8();

        void setAuthValues(Signature const& sig, Public const& remotePubk, h256 const& remoteNonce, uint64_t remoteVersion);
    
        void writeAck();

        void writeAckEIP8();
    
        void readAck();

        void readAckEIP8();
    
        void error();
    
        virtual void transition(boost::system::error_code _ech = boost::system::error_code());

        boost::posix_time::milliseconds const c_timeout = boost::posix_time::milliseconds(1800);

    State m_nextState = New;		    bool m_cancel = false;			    
    Host* m_host;					    
        NodeID m_remote;				    bool m_originated = false;		    
        bytes m_auth;					    bytes m_authCipher;				    bytes m_ack;					    bytes m_ackCipher;				    bytes m_handshakeOutBuffer;		    bytes m_handshakeInBuffer;		    
    KeyPair m_ecdheLocal = KeyPair::create();      h256 m_nonce;					    
    Public m_ecdheRemote;			    h256 m_remoteNonce;				    uint64_t m_remoteVersion;
    
            std::unique_ptr<RLPXFrameCoder> m_io;
    
    std::shared_ptr<RLPXSocket> m_socket;		    boost::asio::deadline_timer m_idleTimer;	
    Logger m_logger{createLogger(VerbosityTrace, "net")};
};
    
}
}
