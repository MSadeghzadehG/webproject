


#pragma once

#include <functional>
#include <mutex>
#include <libdevcore/FileSystem.h>
#include <libdevcore/CommonData.h>
#include <libdevcrypto/SecretStore.h>

#include <boost/filesystem.hpp>

namespace dev
{
namespace eth
{
class PasswordUnknown: public Exception {};

struct KeyInfo
{
	KeyInfo() = default;
	KeyInfo(h256 const& _passHash, std::string const& _accountName, std::string const& _passwordHint = std::string()): passHash(_passHash), accountName(_accountName), passwordHint(_passwordHint) {}

		h256 passHash;
		std::string accountName;
		std::string passwordHint;
};

static h256 const UnknownPassword;
static auto const DontKnowThrow = [](){ throw PasswordUnknown(); return std::string(); };

enum class SemanticPassword
{
	Existing,
	Master
};


class KeyManager
{
public:
	enum class NewKeyType { DirectICAP = 0, NoVanity, FirstTwo, FirstTwoNextTwo, FirstThree, FirstFour };

	KeyManager(boost::filesystem::path const& _keysFile = defaultPath(), boost::filesystem::path const& _secretsPath = SecretStore::defaultPath());
	~KeyManager();

	void setKeysFile(boost::filesystem::path const& _keysFile) { m_keysFile = _keysFile; }
	boost::filesystem::path const& keysFile() const { return m_keysFile; }

	bool exists() const;
	void create(std::string const& _pass);
	bool load(std::string const& _pass);
	void save(std::string const& _pass) const { write(_pass, m_keysFile); }

	void notePassword(std::string const& _pass) { m_cachedPasswords[hashPassword(_pass)] = _pass; }
	void noteHint(std::string const& _pass, std::string const& _hint) { if (!_hint.empty()) m_passwordHint[hashPassword(_pass)] = _hint; }
	bool haveHint(std::string const& _pass) const { auto h = hashPassword(_pass); return m_cachedPasswords.count(h) && !m_cachedPasswords.at(h).empty(); }

		Addresses accounts() const;
		AddressHash accountsHash() const { return AddressHash() + accounts(); }
	bool hasAccount(Address const& _address) const;
		std::string const& accountName(Address const& _address) const;
		std::string const& passwordHint(Address const& _address) const;

		h128 uuid(Address const& _a) const;
		Address address(h128 const& _uuid) const;

	h128 import(Secret const& _s, std::string const& _accountName, std::string const& _pass, std::string const& _passwordHint);
	h128 import(Secret const& _s, std::string const& _accountName) { return import(_s, _accountName, defaultPassword(), std::string()); }

	SecretStore& store() { return m_store; }
	void importExisting(h128 const& _uuid, std::string const& _accountName, std::string const& _pass, std::string const& _passwordHint);
	void importExisting(h128 const& _uuid, std::string const& _accountName, Address const& _addr, h256 const& _passHash = h256(), std::string const& _passwordHint = std::string());

			Secret secret(Address const& _address, std::function<std::string()> const& _pass = DontKnowThrow, bool _usePasswordCache = true) const;
			Secret secret(h128 const& _uuid, std::function<std::string()> const& _pass = DontKnowThrow, bool _usePasswordCache = true) const;

	bool recode(Address const& _address, std::string const& _newPass, std::string const& _hint, std::function<std::string()> const& _pass = DontKnowThrow, KDF _kdf = KDF::Scrypt);

	void kill(h128 const& _id) { kill(address(_id)); }
	void kill(Address const& _a);

	static boost::filesystem::path defaultPath() { return getDataDir("ethereum") / boost::filesystem::path("keys.info"); }

		static KeyPair presaleSecret(std::string const& _json, std::function<std::string(bool)> const& _password);

		static  KeyPair newKeyPair(NewKeyType _type);
private:
	std::string getPassword(h128 const& _uuid, std::function<std::string()> const& _pass = DontKnowThrow) const;
	std::string getPassword(h256 const& _passHash, std::function<std::string()> const& _pass = DontKnowThrow) const;
	std::string defaultPassword(std::function<std::string()> const& _pass = DontKnowThrow) const { return getPassword(m_master, _pass); }
	h256 hashPassword(std::string const& _pass) const;

		void cachePassword(std::string const& _password) const;

			bool write() const { return write(m_keysFile); }
	bool write(boost::filesystem::path const& _keysFile) const;
	void write(std::string const& _pass, boost::filesystem::path const& _keysFile) const;		void write(SecureFixedHash<16> const& _key, boost::filesystem::path const& _keysFile) const;

	
		std::unordered_map<h128, Address> m_uuidLookup;
		std::unordered_map<Address, h128> m_addrLookup;
		std::unordered_map<Address, KeyInfo> m_keyInfo;
		std::unordered_map<h256, std::string> m_passwordHint;

		mutable std::unordered_map<h256, std::string> m_cachedPasswords;

						std::string m_defaultPasswordDeprecated;

	mutable boost::filesystem::path m_keysFile;
	mutable SecureFixedHash<16> m_keysFileKey;
	mutable h256 m_master;
	SecretStore m_store;
};

}
}
