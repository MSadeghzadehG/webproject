


#pragma once

#include <thread>
#include <mutex>
#include <list>
#include <atomic>
#include <boost/asio.hpp> #include <boost/utility.hpp>
#include <libdevcore/Common.h>
#include <libdevcore/CommonIO.h>
#include <libdevcore/Guards.h>
#include <libdevcore/Exceptions.h>
#include <libp2p/Host.h>
#include <libethereum/Client.h>
#include <libethereum/ChainParams.h>

namespace dev
{

enum WorkState
{
    Active = 0,
    Deleting,
    Deleted
};

namespace eth { class Interface; }
namespace shh { class Interface; }
namespace bzz { class Interface; class Client; }

class NetworkFace
{
public:
    virtual ~NetworkFace() = default;

        virtual p2p::NodeInfo nodeInfo() const = 0;

        virtual std::vector<p2p::PeerSessionInfo> peers() = 0;

        virtual size_t peerCount() const = 0;

        virtual void addPeer(p2p::NodeSpec const& _node, p2p::PeerType _t) = 0;

        virtual void addNode(p2p::NodeID const& _node, bi::tcp::endpoint const& _hostEndpoint) = 0;
    
        virtual void requirePeer(p2p::NodeID const& _node, bi::tcp::endpoint const& _endpoint) = 0;
    
        virtual dev::bytes saveNetwork() = 0;

        virtual void setIdealPeerCount(size_t _n) = 0;

    virtual bool haveNetwork() const = 0;

    virtual p2p::NetworkPreferences const& networkPreferences() const = 0;
    virtual void setNetworkPreferences(p2p::NetworkPreferences const& _n, bool _dropPeers) = 0;

    virtual p2p::NodeID id() const = 0;

        virtual u256 networkId() const = 0;

        virtual p2p::Peers nodes() const = 0;

        virtual void startNetwork() = 0;

        virtual void stopNetwork() = 0;

        virtual bool isNetworkStarted() const = 0;

        virtual std::string enode() const = 0;
};



class WebThreeDirect: public NetworkFace
{
public:
            WebThreeDirect(std::string const& _clientVersion, boost::filesystem::path const& _dbPath,
        boost::filesystem::path const& _snapshotPath, eth::ChainParams const& _params,
        WithExisting _we = WithExisting::Trust,
        std::set<std::string> const& _interfaces = {"eth", "shh", "bzz"},
        p2p::NetworkPreferences const& _n = p2p::NetworkPreferences(),
        bytesConstRef _network = bytesConstRef(), bool _testing = false);

        ~WebThreeDirect() override;

    
    eth::Client* ethereum() const
    {
        if (!m_ethereum)
            BOOST_THROW_EXCEPTION(InterfaceNotSupported() << errinfo_interface("eth"));
        return m_ethereum.get();
    }

    
    static std::string composeClientVersion(std::string const& _client);
    std::string const& clientVersion() const { return m_clientVersion; }

    
        std::vector<p2p::PeerSessionInfo> peers() override;

        size_t peerCount() const override;
    
        virtual void addPeer(p2p::NodeSpec const& _node, p2p::PeerType _t) override;

        virtual void addNode(p2p::NodeID const& _node, bi::tcp::endpoint const& _hostEndpoint) override;

        void addNode(p2p::NodeID const& _node, std::string const& _hostString) { addNode(_node, p2p::Network::resolveHost(_hostString)); }
    
        void addNode(bi::tcp::endpoint const& _endpoint) { addNode(p2p::NodeID(), _endpoint); }

        void addNode(std::string const& _hostString) { addNode(p2p::NodeID(), _hostString); }
    
        void requirePeer(p2p::NodeID const& _node, bi::tcp::endpoint const& _endpoint) override;

        void requirePeer(p2p::NodeID const& _node, std::string const& _hostString) { requirePeer(_node, p2p::Network::resolveHost(_hostString)); }

        dev::bytes saveNetwork() override;

        void setIdealPeerCount(size_t _n) override;

        void setPeerStretch(size_t _n);
    
    bool haveNetwork() const override { return m_net.haveNetwork(); }

    p2p::NetworkPreferences const& networkPreferences() const override;

    void setNetworkPreferences(p2p::NetworkPreferences const& _n, bool _dropPeers = false) override;

    p2p::NodeInfo nodeInfo() const override { return m_net.nodeInfo(); }

    p2p::NodeID id() const override { return m_net.id(); }

    u256 networkId() const override { return m_ethereum.get()->networkId(); }

    std::string enode() const override { return m_net.enode(); }

        p2p::Peers nodes() const override { return m_net.getPeers(); }

        void startNetwork() override { m_net.start(); }

        void stopNetwork() override { m_net.stop(); }

        bool isNetworkStarted() const override { return m_net.isStarted(); }

private:
    std::string m_clientVersion;                    
    std::unique_ptr<eth::Client> m_ethereum;        
    p2p::Host m_net;                                };


}
