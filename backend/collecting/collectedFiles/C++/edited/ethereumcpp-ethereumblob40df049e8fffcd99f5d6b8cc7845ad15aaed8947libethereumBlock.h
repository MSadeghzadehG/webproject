


#pragma once

#include <array>
#include <unordered_map>
#include <libdevcore/Common.h>
#include <libdevcore/RLP.h>
#include <libdevcore/TrieDB.h>
#include <libdevcore/OverlayDB.h>
#include <libethcore/Exceptions.h>
#include <libethcore/BlockHeader.h>
#include <libethcore/ChainOperationParams.h>
#include "Account.h"
#include "Transaction.h"
#include "TransactionReceipt.h"
#include "GasPricer.h"
#include "State.h"

namespace dev
{

namespace test { class ImportTest; class StateLoader; }

namespace eth
{

class SealEngineFace;
class BlockChain;
class State;
class TransactionQueue;
struct VerifiedBlockRef;
class LastBlockHashesFace;

struct PopulationStatistics
{
    double verify;
    double enact;
};

DEV_SIMPLE_EXCEPTION(ChainOperationWithUnknownBlockChain);
DEV_SIMPLE_EXCEPTION(InvalidOperationOnSealedBlock);


class Block
{
    friend class ExtVM;
    friend class dev::test::ImportTest;
    friend class dev::test::StateLoader;
    friend class Executive;
    friend class BlockChain;

public:
    
        Block(u256 const& _accountStartNonce): m_state(_accountStartNonce, OverlayDB(), BaseState::Empty), m_precommit(_accountStartNonce) {}

                        Block(BlockChain const& _bc, OverlayDB const& _db, BaseState _bs = BaseState::PreExisting, Address const& _author = Address());

                        Block(BlockChain const& _bc, OverlayDB const& _db, h256 const& _root, Address const& _author = Address());

    enum NullType { Null };
    Block(NullType): m_state(0, OverlayDB(), BaseState::Empty), m_precommit(0) {}

        explicit Block(BlockChain const& _bc): Block(Null) { noteChain(_bc); }

        Block(Block const& _s);

        Block& operator=(Block const& _s);

        Address author() const { return m_author; }

            void setAuthor(Address const& _id) { m_author = _id; resetCurrent(); }

            void noteChain(BlockChain const& _bc);

    
            u256 balance(Address const& _address) const { return m_state.balance(_address); }

            u256 transactionsFrom(Address const& _address) const { return m_state.getNonce(_address); }

        bool addressInUse(Address const& _address) const { return m_state.addressInUse(_address); }

        bool addressHasCode(Address const& _address) const { return m_state.addressHasCode(_address); }

        h256 storageRoot(Address const& _contract) const { return m_state.storageRoot(_contract); }

            u256 storage(Address const& _contract, u256 const& _memory) const { return m_state.storage(_contract, _memory); }

                std::map<h256, std::pair<u256, u256>> storage(Address const& _contract) const { return m_state.storage(_contract); }

            bytes const& code(Address const& _contract) const { return m_state.code(_contract); }

            h256 codeHash(Address const& _contract) const { return m_state.codeHash(_contract); }

    
        State const& state() const { return m_state; }

        OverlayDB const& db() const { return m_state.db(); }

        h256 rootHash() const { return m_state.rootHash(); }

            std::unordered_map<Address, u256> addresses() const { return m_state.addresses(); }

    
                State& mutableState() { return m_state; }

    
        u256 gasLimitRemaining() const { return m_currentBlock.gasLimit() - gasUsed(); }

        Transactions const& pending() const { return m_transactions; }

        h256Hash const& pendingHashes() const { return m_transactionSet; }

        TransactionReceipt const& receipt(unsigned _i) const { return m_receipts.at(_i); }

        LogEntries const& log(unsigned _i) const { return receipt(_i).log(); }

        LogBloom logBloom() const;

        LogBloom const& logBloom(unsigned _i) const { return receipt(_i).bloom(); }

                    h256 stateRootBeforeTx(unsigned _i) const;

    
        PopulationStatistics populateFromChain(BlockChain const& _bc, h256 const& _hash, ImportRequirements::value _ir = ImportRequirements::None);

            ExecutionResult execute(LastBlockHashesFace const& _lh, Transaction const& _t, Permanence _p = Permanence::Committed, OnOpFunc const& _onOp = OnOpFunc());

            std::pair<TransactionReceipts, bool> sync(BlockChain const& _bc, TransactionQueue& _tq, GasPricer const& _gp, unsigned _msTimeout = 100);

            bool sync(BlockChain const& _bc);

        bool sync(BlockChain const& _bc, h256 const& _blockHash, BlockHeader const& _bi = BlockHeader());

            u256 enactOn(VerifiedBlockRef const& _block, BlockChain const& _bc);

        void cleanup();

            void resetCurrent(int64_t _timestamp = utcTime());

    
                            void commitToSeal(BlockChain const& _bc, bytes const& _extraData = {});

                    
    bool sealBlock(bytes const& _header) { return sealBlock(&_header); }
    bool sealBlock(bytesConstRef _header);

        bool isSealed() const { return !m_currentBytes.empty(); }

            bytes const& blockData() const { return m_currentBytes; }

        BlockHeader const& info() const { return m_currentBlock; }

private:
    SealEngineFace* sealEngine() const;

        void uncommitToSeal();

            u256 enact(VerifiedBlockRef const& _block, BlockChain const& _bc);

        void applyRewards(std::vector<BlockHeader> const& _uncleBlockHeaders, u256 const& _blockReward);

        u256 gasUsed() const { return m_receipts.size() ? m_receipts.back().cumulativeGasUsed() : 0; }

        void performIrregularModifications();

        void updateBlockhashContract();

    State m_state;								    Transactions m_transactions;				    TransactionReceipts m_receipts;				    h256Hash m_transactionSet;					    State m_precommit;							
    BlockHeader m_previousBlock;				    BlockHeader m_currentBlock;					    bytes m_currentBytes;						    bool m_committedToSeal = false;				
    bytes m_currentTxs;							    bytes m_currentUncles;						
    Address m_author;							
    SealEngineFace* m_sealEngine = nullptr;		
    Logger m_logger{createLogger(VerbosityDebug, "block")};
    Logger m_loggerDetailed{createLogger(VerbosityTrace, "block")};
};


}

}
