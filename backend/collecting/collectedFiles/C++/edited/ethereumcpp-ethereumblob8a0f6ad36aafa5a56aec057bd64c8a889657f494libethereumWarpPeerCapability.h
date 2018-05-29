

#pragma once

#include "CommonNet.h"

#include <libdevcore/Common.h>
#include <libp2p/Capability.h>

namespace dev
{
namespace eth
{
class SnapshotStorageFace;
class WarpPeerCapability;

const unsigned c_WarpProtocolVersion = 1;

enum WarpSubprotocolPacketType : byte
{
    WarpStatusPacket = 0x00,
    GetSnapshotManifest = 0x11,
    SnapshotManifest = 0x12,
    GetSnapshotData = 0x13,
    SnapshotData = 0x14,

    WarpSubprotocolPacketCount
};

class WarpPeerObserverFace
{
public:
    virtual ~WarpPeerObserverFace() {}

    virtual void onPeerStatus(std::shared_ptr<WarpPeerCapability> _peer) = 0;

    virtual void onPeerManifest(std::shared_ptr<WarpPeerCapability> _peer, RLP const& _r) = 0;

    virtual void onPeerBlockHeaders(std::shared_ptr<WarpPeerCapability> _peer, RLP const& _r) = 0;

    virtual void onPeerData(std::shared_ptr<WarpPeerCapability> _peer, RLP const& _r) = 0;

    virtual void onPeerDisconnect(std::shared_ptr<WarpPeerCapability> _peer, Asking _asking) = 0;
};

class WarpPeerCapability : public p2p::Capability
{
public:
    WarpPeerCapability(std::shared_ptr<p2p::SessionFace> _s, p2p::HostCapabilityFace* _h,
        unsigned _i, p2p::CapDesc const& _cap);

    static std::string name() { return "par"; }

    static u256 version() { return c_WarpProtocolVersion; }

    static unsigned messageCount() { return WarpSubprotocolPacketCount; }

    void init(unsigned _hostProtocolVersion, u256 _hostNetworkId, u256 _chainTotalDifficulty,
        h256 _chainCurrentHash, h256 _chainGenesisHash,
        std::shared_ptr<SnapshotStorageFace const> _snapshot,
        std::weak_ptr<WarpPeerObserverFace> _observer);

        bool validateStatus(h256 const& _genesisHash, std::vector<unsigned> const& _protocolVersions,
        u256 const& _networkId);

    void requestStatus(unsigned _hostProtocolVersion, u256 const& _hostNetworkId,
        u256 const& _chainTotalDifficulty, h256 const& _chainCurrentHash,
        h256 const& _chainGenesisHash, h256 const& _snapshotBlockHash,
        u256 const& _snapshotBlockNumber);
    void requestBlockHeaders(unsigned _startNumber, unsigned _count, unsigned _skip, bool _reverse);
    void requestManifest();
    void requestData(h256 const& _chunkHash);

        void tick();

    u256 snapshotNumber() const { return m_snapshotNumber; }

    using p2p::Capability::disable;

private:
    using p2p::Capability::sealAndSend;

    bool interpret(unsigned _id, RLP const& _r) override;

    void onDisconnect() override;

    void setAsking(Asking _a);

    void setIdle() { setAsking(Asking::Nothing); }

        unsigned m_protocolVersion = 0;

        u256 m_networkId;

        Asking m_asking = Asking::Nothing;
        std::atomic<time_t> m_lastAsk;

        h256 m_latestHash;           u256 m_totalDifficulty;      h256 m_genesisHash;          h256 m_snapshotHash;
    u256 m_snapshotNumber;

    std::shared_ptr<SnapshotStorageFace const> m_snapshot;
    std::weak_ptr<WarpPeerObserverFace> m_observer;
};

}  }  