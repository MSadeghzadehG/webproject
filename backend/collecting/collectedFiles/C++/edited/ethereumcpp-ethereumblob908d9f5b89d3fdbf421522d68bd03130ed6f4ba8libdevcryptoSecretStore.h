


#pragma once

#include <functional>
#include <mutex>
#include <libdevcore/FixedHash.h>
#include <libdevcore/FileSystem.h>
#include "Common.h"

#include <boost/filesystem.hpp>

namespace dev
{

enum class KDF {
	PBKDF2_SHA256,
	Scrypt,
};


class SecretStore
{
public:
	struct EncryptedKey
	{
		std::string encryptedKey;
		boost::filesystem::path filename;
		Address address;
	};

			SecretStore() = default;

		SecretStore(boost::filesystem::path const& _path);

		void setPath(boost::filesystem::path const& _path);

				bytesSec secret(h128 const& _uuid, std::function<std::string()> const& _pass, bool _useCache = true) const;
			static bytesSec secret(std::string const& _content, std::string const& _pass);
			bytesSec secret(Address const& _address, std::function<std::string()> const& _pass) const;
		h128 importKey(std::string const& _file) { auto ret = readKey(_file, false); if (ret) save(); return ret; }
			h128 importKeyContent(std::string const& _content) { auto ret = readKeyContent(_content, std::string()); if (ret) save(); return ret; }
			h128 importSecret(bytesSec const& _s, std::string const& _pass);
	h128 importSecret(bytesConstRef _s, std::string const& _pass);
		bool recode(h128 const& _uuid, std::string const& _newPass, std::function<std::string()> const& _pass, KDF _kdf = KDF::Scrypt);
		bool recode(Address const& _address, std::string const& _newPass, std::function<std::string()> const& _pass, KDF _kdf = KDF::Scrypt);
		void kill(h128 const& _uuid);

		std::vector<h128> keys() const { return keysOf(m_keys); }

		bool contains(h128 const& _k) const { return m_keys.count(_k); }

			void clearCache() const;

				h128 readKey(boost::filesystem::path const& _file, bool _takeFileOwnership);
					h128 readKeyContent(std::string const& _content, boost::filesystem::path const& _file = boost::filesystem::path());

		void save(boost::filesystem::path const& _keysPath);
		void save() { save(m_path); }
		bool noteAddress(h128 const& _uuid, Address const& _address);
		Address address(h128 const& _uuid) const { return m_keys.at(_uuid).address; }

		static boost::filesystem::path defaultPath() { return getDataDir("web3") / boost::filesystem::path("keys"); }

private:
		void load(boost::filesystem::path const& _keysPath);
	void load() { load(m_path); }
		static std::string encrypt(bytesConstRef _v, std::string const& _pass, KDF _kdf = KDF::Scrypt);
		static bytesSec decrypt(std::string const& _v, std::string const& _pass);
		std::pair<h128 const, EncryptedKey> const* key(Address const& _address) const;
	std::pair<h128 const, EncryptedKey>* key(Address const& _address);
		mutable std::unordered_map<h128, bytesSec> m_cached;
		std::unordered_map<h128, EncryptedKey> m_keys;

	boost::filesystem::path m_path;
};

}

