


#pragma once

#include <libdevcore/Common.h>
#include <libdevcore/RLP.h>
#include <libdevcore/TrieDB.h>
#include <libdevcore/SHA3.h>
#include <libethcore/Common.h>

namespace dev
{
namespace eth
{


class Account
{
public:
		enum Changedness
	{
				Changed,
				Unchanged
	};

		Account() {}

				Account(u256 _nonce, u256 _balance, Changedness _c = Changed): m_isAlive(true), m_isUnchanged(_c == Unchanged), m_nonce(_nonce), m_balance(_balance) {}

		Account(u256 _nonce, u256 _balance, h256 _contractRoot, h256 _codeHash, Changedness _c): m_isAlive(true), m_isUnchanged(_c == Unchanged), m_nonce(_nonce), m_balance(_balance), m_storageRoot(_contractRoot), m_codeHash(_codeHash) { assert(_contractRoot); }


		void kill() { m_isAlive = false; m_storageOverlay.clear(); m_codeHash = EmptySHA3; m_storageRoot = EmptyTrie; m_balance = 0; m_nonce = 0; changed(); }

				bool isAlive() const { return m_isAlive; }

		bool isDirty() const { return !m_isUnchanged; }

	void untouch() { m_isUnchanged = true; }

			bool isEmpty() const { return nonce() == 0 && balance() == 0 && codeHash() == EmptySHA3; }

		u256 const& balance() const { return m_balance; }

		void addBalance(u256 _value) { m_balance += _value; changed(); }

		u256 nonce() const { return m_nonce; }

		void incNonce() { ++m_nonce; changed(); }

			void setNonce(u256 const& _nonce) { m_nonce = _nonce; changed(); }


			h256 baseRoot() const { assert(m_storageRoot); return m_storageRoot; }

		std::unordered_map<u256, u256> const& storageOverlay() const { return m_storageOverlay; }

			void setStorage(u256 _p, u256 _v) { m_storageOverlay[_p] = _v; changed(); }

		void clearStorage() { m_storageOverlay.clear(); m_storageRoot = EmptyTrie; changed(); }

		void setStorageRoot(h256 const& _root) { m_storageOverlay.clear(); m_storageRoot = _root; changed(); }

			void setStorageCache(u256 _p, u256 _v) const { const_cast<decltype(m_storageOverlay)&>(m_storageOverlay)[_p] = _v; }

		h256 codeHash() const { return m_codeHash; }

	bool hasNewCode() const { return m_hasNewCode; }

		void setCode(bytes&& _code);

		void resetCode() { m_codeCache.clear(); m_hasNewCode = false; m_codeHash = EmptySHA3; }

			void noteCode(bytesConstRef _code) { assert(sha3(_code) == m_codeHash); m_codeCache = _code.toBytes(); }

		bytes const& code() const { return m_codeCache; }

private:
		void changed() { m_isUnchanged = false; }

		bool m_isAlive = false;

		bool m_isUnchanged = false;

		bool m_hasNewCode = false;

		u256 m_nonce;

		u256 m_balance = 0;

			h256 m_storageRoot = EmptyTrie;

	
	h256 m_codeHash = EmptySHA3;

		std::unordered_map<u256, u256> m_storageOverlay;

			bytes m_codeCache;

		static const h256 c_contractConceptionCodeHash;
};

class AccountMask
{
public:
	AccountMask(bool _all = false):
		m_hasBalance(_all),
		m_hasNonce(_all),
		m_hasCode(_all),
		m_hasStorage(_all)
	{}

	AccountMask(
		bool _hasBalance,
		bool _hasNonce,
		bool _hasCode,
		bool _hasStorage,
		bool _shouldNotExist = false
	):
		m_hasBalance(_hasBalance),
		m_hasNonce(_hasNonce),
		m_hasCode(_hasCode),
		m_hasStorage(_hasStorage),
		m_shouldNotExist(_shouldNotExist)
	{}

	bool allSet() const { return m_hasBalance && m_hasNonce && m_hasCode && m_hasStorage; }
	bool hasBalance() const { return m_hasBalance; }
	bool hasNonce() const { return m_hasNonce; }
	bool hasCode() const { return m_hasCode; }
	bool hasStorage() const { return m_hasStorage; }
	bool shouldExist() const { return !m_shouldNotExist; }

private:
	bool m_hasBalance;
	bool m_hasNonce;
	bool m_hasCode;
	bool m_hasStorage;
	bool m_shouldNotExist = false;
};

using AccountMap = std::unordered_map<Address, Account>;
using AccountMaskMap = std::unordered_map<Address, AccountMask>;

class PrecompiledContract;
using PrecompiledContractMap = std::unordered_map<Address, PrecompiledContract>;

AccountMap jsonToAccountMap(std::string const& _json, u256 const& _defaultNonce = 0,
    AccountMaskMap* o_mask = nullptr, PrecompiledContractMap* o_precompiled = nullptr,
    const boost::filesystem::path& _configPath = {});
}
}
