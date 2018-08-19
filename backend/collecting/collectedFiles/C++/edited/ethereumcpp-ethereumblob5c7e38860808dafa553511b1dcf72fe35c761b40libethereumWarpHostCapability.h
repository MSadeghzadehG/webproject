

#pragma once

#include "WarpPeerCapability.h"

#include <libdevcore/Worker.h>

namespace dev
{
namespace eth
{
class WarpHostCapability : public p2p::HostCapability<WarpPeerCapability>, Worker
{
public:
    WarpHostCapability(BlockChain const& _blockChain, u256 const& _networkId,
        boost::filesystem::path const& _snapshotDownloadPath,
        std::shared_ptr<SnapshotStorageFace> _snapshotStorage);
    ~WarpHostCapability();

    unsigned protocolVersion() const { return c_WarpProtocolVersion; }
    u256 networkId() const { return m_networkId; }

protected:
    std::shared_ptr<p2p::Capability> newPeerCapability(std::shared_ptr<p2p::SessionFace> const& _s,
        unsigned _idOffset, p2p::CapDesc const& _cap) override;

private:
    std::shared_ptr<WarpPeerObserverFace> createPeerObserver(
        boost::filesystem::path const& _snapshotDownloadPath) const;

    void doWork() override;

    BlockChain const& m_blockChain;
    u256 const m_networkId;

    std::shared_ptr<SnapshotStorageFace> m_snapshot;
    std::shared_ptr<WarpPeerObserverFace> m_peerObserver;
    time_t m_lastTick;
};

}  }  