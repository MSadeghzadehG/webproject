

#pragma once

#include <array>
#include <unordered_map>
#include <libdevcore/Common.h>
#include <libdevcore/RLP.h>
#include <libdevcore/TrieDB.h>
#include <libdevcore/OverlayDB.h>
#include <libethcore/Exceptions.h>
#include <libethcore/BlockHeader.h>
#include <libethereum/CodeSizeCache.h>
#include <libevm/ExtVMFace.h>
#include "Account.h"
#include "Transaction.h"
#include "TransactionReceipt.h"
#include "GasPricer.h"

namespace dev
{

namespace test { class ImportTest; class StateLoader; }

namespace eth
{

using errinfo_uncleIndex = boost::error_info<struct tag_uncleIndex, unsigned>;
using errinfo_currentNumber = boost::error_info<struct tag_currentNumber, u256>;
using errinfo_uncleNumber = boost::error_info<struct tag_uncleNumber, u256>;
using errinfo_unclesExcluded = boost::error_info<struct tag_unclesExcluded, h256Hash>;
using errinfo_block = boost::error_info<struct tag_block, bytes>;
using errinfo_now = boost::error_info<struct tag_now, unsigned>;

using errinfo_transactionIndex = boost::error_info<struct tag_transactionIndex, unsigned>;

using errinfo_vmtrace = boost::error_info<struct tag_vmtrace, std::string>;
using errinfo_receipts = boost::error_info<struct tag_receipts, std::vector<bytes>>;
using errinfo_transaction = boost::error_info<struct tag_transaction, bytes>;
using errinfo_phase = boost::error_info<struct tag_phase, unsigned>;
using errinfo_required_LogBloom = boost::error_info<struct tag_required_LogBloom, LogBloom>;
using errinfo_got_LogBloom = boost::error_info<struct tag_get_LogBloom, LogBloom>;
using LogBloomRequirementError = boost::tuple<errinfo_required_LogBloom, errinfo_got_LogBloom>;

class BlockChain;
class State;
class TransactionQueue;
struct VerifiedBlockRef;

enum class BaseState
{
    PreExisting,
    Empty
};

enum class Permanence
{
    Reverted,
    Committed,
    Uncommitted	};

#if ETH_FATDB
template <class KeyType, class DB> using SecureTrieDB = SpecificTrieDB<FatGenericTrieDB<DB>, KeyType>;
#else
template <class KeyType, class DB> using SecureTrieDB = SpecificTrieDB<HashedGenericTrieDB<DB>, KeyType>;
#endif

DEV_SIMPLE_EXCEPTION(InvalidAccountStartNonceInState);
DEV_SIMPLE_EXCEPTION(IncorrectAccountStartNonceInState);

class SealEngineFace;
class Executive;

struct Change
{
    enum Kind: int
    {
                        Balance,

                        Storage,

                        StorageRoot,

                Nonce,

                Create,

                Code,

                Touch
    };

    Kind kind;            Address address;      u256 value;           u256 key;             bytes oldCode;    
        Change(Kind _kind, Address const& _addr, u256 const& _value = 0):
            kind(_kind), address(_addr), value(_value)
    {
        assert(_kind != Code);     }

        Change(Address const& _addr, u256 const& _key, u256 const& _value):
            kind(Storage), address(_addr), value(_value), key(_key)
    {}

        Change(Address const& _addr, u256 const& _value):
            kind(Nonce), address(_addr), value(_value)
    {}

        Change(Address const& _addr, bytes const& _oldCode):
            kind(Code), address(_addr), oldCode(_oldCode)
    {}
};

using ChangeLog = std::vector<Change>;


class State
{
    friend class ExtVM;
    friend class dev::test::ImportTest;
    friend class dev::test::StateLoader;
    friend class BlockChain;

public:
    enum class CommitBehaviour
    {
        KeepEmptyAccounts,
        RemoveEmptyAccounts
    };

    using AddressMap = std::map<h256, Address>;

        explicit State(u256 const& _accountStartNonce): State(_accountStartNonce, OverlayDB(), BaseState::Empty) {}

                    explicit State(u256 const& _accountStartNonce, OverlayDB const& _db, BaseState _bs = BaseState::PreExisting);

    enum NullType { Null };
    State(NullType): State(Invalid256, OverlayDB(), BaseState::Empty) {}

        State(State const& _s);

        State& operator=(State const& _s);

        static OverlayDB openDB(boost::filesystem::path const& _path, h256 const& _genesisHash, WithExisting _we = WithExisting::Trust);
    OverlayDB const& db() const { return m_db; }
    OverlayDB& db() { return m_db; }

        void populateFrom(AccountMap const& _map);

                std::unordered_map<Address, u256> addresses() const;

            std::pair<AddressMap, h256> addresses(h256 const& _begin, size_t _maxResults) const;

            std::pair<ExecutionResult, TransactionReceipt> execute(EnvInfo const& _envInfo, SealEngineFace const& _sealEngine, Transaction const& _t, Permanence _p = Permanence::Committed, OnOpFunc const& _onOp = OnOpFunc());

            void executeBlockTransactions(Block const& _block, unsigned _txCount, LastBlockHashesFace const& _lastHashes, SealEngineFace const& _sealEngine);

        bool addressInUse(Address const& _address) const;

            bool accountNonemptyAndExisting(Address const& _address) const;

        bool addressHasCode(Address const& _address) const;

            u256 balance(Address const& _id) const;

            void addBalance(Address const& _id, u256 const& _amount);

                void subBalance(Address const& _addr, u256 const& _value);

            void setBalance(Address const& _addr, u256 const& _value);

    
    void transferBalance(Address const& _from, Address const& _to, u256 const& _value) { subBalance(_from, _value); addBalance(_to, _value); }

        h256 storageRoot(Address const& _contract) const;

            u256 storage(Address const& _contract, u256 const& _memory) const;

        void setStorage(Address const& _contract, u256 const& _location, u256 const& _value);

        void clearStorage(Address const& _contract);

        void createContract(Address const& _address);

        void setCode(Address const& _address, bytes&& _code);

        void kill(Address _a);

                std::map<h256, std::pair<u256, u256>> storage(Address const& _contract) const;

                    bytes const& code(Address const& _addr) const;

            h256 codeHash(Address const& _contract) const;

            size_t codeSize(Address const& _contract) const;

        void incNonce(Address const& _id);

        void setNonce(Address const& _addr, u256 const& _newNonce);

            u256 getNonce(Address const& _addr) const;

        h256 rootHash() const { return m_state.root(); }

            void commit(CommitBehaviour _commitBehaviour);

        void setRoot(h256 const& _root);

        u256 const& accountStartNonce() const { return m_accountStartNonce; }
    u256 const& requireAccountStartNonce() const;
    void noteAccountStartNonce(u256 const& _actual);

            size_t savepoint() const;

        void rollback(size_t _savepoint);

    ChangeLog const& changeLog() const { return m_changeLog; }

private:
        void removeEmptyAccounts();

            Account const* account(Address const& _addr) const;

            Account* account(Address const& _addr);

        void clearCacheIfTooLarge() const;

    void createAccount(Address const& _address, Account const&& _account);

    OverlayDB m_db;								    SecureTrieDB<Address, OverlayDB> m_state;	    mutable std::unordered_map<Address, Account> m_cache;	    mutable std::vector<Address> m_unchangedCacheEntries;	    mutable std::set<Address> m_nonExistingAccountsCache;	    AddressHash m_touched;						
    u256 m_accountStartNonce;

    friend std::ostream& operator<<(std::ostream& _out, State const& _s);
    ChangeLog m_changeLog;
};

std::ostream& operator<<(std::ostream& _out, State const& _s);

State& createIntermediateState(State& o_s, Block const& _block, unsigned _txIndex, BlockChain const& _bc);

template <class DB>
AddressHash commit(AccountMap const& _cache, SecureTrieDB<Address, DB>& _state);

}
}

