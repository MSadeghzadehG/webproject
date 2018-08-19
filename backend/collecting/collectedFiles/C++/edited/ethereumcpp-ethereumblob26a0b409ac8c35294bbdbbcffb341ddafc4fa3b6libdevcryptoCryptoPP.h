


#pragma once

#include "Common.h"

namespace dev
{
namespace crypto
{
static const unsigned c_eciesOverhead = 113;


class Secp256k1PP
{	
public:
	static Secp256k1PP* get();

		void encrypt(Public const& _k, bytes& io_cipher);
	
		void decrypt(Secret const& _k, bytes& io_text);
	
		void encryptECIES(Public const& _k, bytes& io_cipher);
	
		void encryptECIES(Public const& _k, bytesConstRef _sharedMacData, bytes& io_cipher);
	
		bool decryptECIES(Secret const& _k, bytes& io_text);
	
		bool decryptECIES(Secret const& _k, bytesConstRef _sharedMacData, bytes& io_text);

private:
	Secp256k1PP() = default;
};

}
}

