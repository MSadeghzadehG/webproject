


#pragma once

#include <mutex>
#include <unordered_map>
#include <vector>
#include <unordered_set>
#include <memory>
#include <utility>
#include <thread>

#include <libdevcore/Guards.h>
#include <libdevcore/Worker.h>
#include <libethcore/Common.h>
#include <libp2p/Common.h>
#include <libdevcore/OverlayDB.h>
#include <libethcore/BlockHeader.h>
#include <libethereum/BlockChainSync.h>
#include "CommonNet.h"
#include "EthereumPeer.h"

namespace dev
{

class RLPStream;

namespace eth
{

class TransactionQueue;
class BlockQueue;
class BlockChainSync;


class EthereumHost: public p2p::HostCapability<EthereumPeer>, Worker
{
public:
        EthereumHost(BlockChain const& _ch, OverlayDB const& _db, TransactionQueue& _tq, BlockQueue& _bq, u256 _networkId);

        virtual ~EthereumHost();

    unsigned protocolVersion() const { return c_protocolVersion; }
    u256 networkId() const { return m_networkId; }
    void setNetworkId(u256 _n) { m_networkId = _n; }

    void reset();
        void completeSync();

    bool isSyncing() const;
    bool isBanned(p2p::NodeID const& _id) const { return !!m_banned.count(_id); }

    void noteNewTransactions() { m_newTransactions = true; }
    void noteNewBlocks() { m_newBlocks = true; }
    void onBlockImported(BlockHeader const& _info) { m_sync->onBlockImported(_info); }

    BlockChain const& chain() const { return m_chain; }
    OverlayDB const& db() const { return m_db; }
    BlockQueue& bq() { return m_bq; }
    BlockQueue const& bq() const { return m_bq; }
    SyncStatus status() const;
    h256 latestBlockSent() { return m_latestBlockSent; }
    static char const* stateName(SyncState _s) { return s_stateNames[static_cast<int>(_s)]; }

    static unsigned const c_oldProtocolVersion;
    void foreachPeer(std::function<bool(std::shared_ptr<EthereumPeer>)> const& _f) const;

protected:
    std::shared_ptr<p2p::Capability> newPeerCapability(std::shared_ptr<p2p::SessionFace> const& _s, unsigned _idOffset, p2p::CapDesc const& _cap) override;

private:
    static char const* const s_stateNames[static_cast<int>(SyncState::Size)];

    std::tuple<std::vector<std::shared_ptr<EthereumPeer>>, std::vector<std::shared_ptr<EthereumPeer>>, std::vector<std::shared_ptr<p2p::SessionFace>>> randomSelection(unsigned _percent = 25, std::function<bool(EthereumPeer*)> const& _allow = [](EthereumPeer const*){ return true; });

        virtual void doWork() override;

    void maintainTransactions();
    void maintainBlocks(h256 const& _currentBlock);
    void onTransactionImported(ImportResult _ir, h256 const& _h, h512 const& _nodeId);

        bool isInitialised() const { return (bool)m_latestBlockSent; }

        bool ensureInitialised();

    virtual void onStarting() override { startWorking(); }
    virtual void onStopping() override { stopWorking(); }

    BlockChain const& m_chain;
    OverlayDB const& m_db;					    TransactionQueue& m_tq;					    BlockQueue& m_bq;						
    u256 m_networkId;

    h256 m_latestBlockSent;
    h256Hash m_transactionsSent;

    std::unordered_set<p2p::NodeID> m_banned;

    bool m_newTransactions = false;
    bool m_newBlocks = false;

    mutable Mutex x_transactions;
    std::shared_ptr<BlockChainSync> m_sync;
    std::atomic<time_t> m_lastTick = { 0 };

    std::shared_ptr<EthereumHostDataFace> m_hostData;
    std::shared_ptr<EthereumPeerObserverFace> m_peerObserver;

    Logger m_logger{createLogger(VerbosityDebug, "host")};
};

}
}
