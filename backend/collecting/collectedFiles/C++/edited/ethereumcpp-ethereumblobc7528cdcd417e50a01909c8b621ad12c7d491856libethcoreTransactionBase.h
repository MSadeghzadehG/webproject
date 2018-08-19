

#pragma once

#include <libethcore/Common.h>
#include <libdevcrypto/Common.h>
#include <libdevcore/RLP.h>
#include <libdevcore/SHA3.h>

#include <boost/optional.hpp>

namespace dev
{
namespace eth
{

struct EVMSchedule;

enum IncludeSignature
{
	WithoutSignature = 0,		WithSignature = 1,		};

enum class CheckTransaction
{
	None,
	Cheap,
	Everything
};

class TransactionBase
{
public:
		TransactionBase() {}

		TransactionBase(TransactionSkeleton const& _ts, Secret const& _s = Secret());

		TransactionBase(u256 const& _value, u256 const& _gasPrice, u256 const& _gas, Address const& _dest, bytes const& _data, u256 const& _nonce, Secret const& _secret): m_type(MessageCall), m_nonce(_nonce), m_value(_value), m_receiveAddress(_dest), m_gasPrice(_gasPrice), m_gas(_gas), m_data(_data) { sign(_secret); }

		TransactionBase(u256 const& _value, u256 const& _gasPrice, u256 const& _gas, bytes const& _data, u256 const& _nonce, Secret const& _secret): m_type(ContractCreation), m_nonce(_nonce), m_value(_value), m_gasPrice(_gasPrice), m_gas(_gas), m_data(_data) { sign(_secret); }

		TransactionBase(u256 const& _value, u256 const& _gasPrice, u256 const& _gas, Address const& _dest, bytes const& _data, u256 const& _nonce = 0): m_type(MessageCall), m_nonce(_nonce), m_value(_value), m_receiveAddress(_dest), m_gasPrice(_gasPrice), m_gas(_gas), m_data(_data) {}

		TransactionBase(u256 const& _value, u256 const& _gasPrice, u256 const& _gas, bytes const& _data, u256 const& _nonce = 0): m_type(ContractCreation), m_nonce(_nonce), m_value(_value), m_gasPrice(_gasPrice), m_gas(_gas), m_data(_data) {}

		explicit TransactionBase(bytesConstRef _rlp, CheckTransaction _checkSig);

		explicit TransactionBase(bytes const& _rlp, CheckTransaction _checkSig): TransactionBase(&_rlp, _checkSig) {}

		bool operator==(TransactionBase const& _c) const { return m_type == _c.m_type && (m_type == ContractCreation || m_receiveAddress == _c.m_receiveAddress) && m_value == _c.m_value && m_data == _c.m_data; }
		bool operator!=(TransactionBase const& _c) const { return !operator==(_c); }

			Address const& sender() const;
		Address const& safeSender() const noexcept;
		void forceSender(Address const& _a) { m_sender = _a; }

			void checkLowS() const;

				void checkChainId(int chainId = -4) const;

		explicit operator bool() const { return m_type != NullTransaction; }

		bool isCreation() const { return m_type == ContractCreation; }

			void streamRLP(RLPStream& _s, IncludeSignature _sig = WithSignature, bool _forEip155hash = false) const;

		bytes rlp(IncludeSignature _sig = WithSignature) const { RLPStream s; streamRLP(s, _sig); return s.out(); }

		h256 sha3(IncludeSignature _sig = WithSignature) const;

		u256 value() const { return m_value; }

		u256 gasPrice() const { return m_gasPrice; }

		u256 gas() const { return m_gas; }

		Address receiveAddress() const { return m_receiveAddress; }

		Address to() const { return m_receiveAddress; }

		Address from() const { return safeSender(); }

		bytes const& data() const { return m_data; }

		u256 nonce() const { return m_nonce; }

		void setNonce(u256 const& _n) { clearSignature(); m_nonce = _n; }

		bool hasSignature() const { return m_vrs.is_initialized(); }

		bool hasZeroSignature() const { return m_vrs && isZeroSignature(m_vrs->r, m_vrs->s); }

		bool isReplayProtected() const { return m_chainId != -4; }

			SignatureStruct const& signature() const;

	void sign(Secret const& _priv);			
		int64_t baseGasRequired(EVMSchedule const& _es) const { return baseGasRequired(isCreation(), &m_data, _es); }

		static int64_t baseGasRequired(bool _contractCreation, bytesConstRef _data, EVMSchedule const& _es);

protected:
		enum Type
	{
		NullTransaction,						ContractCreation,						MessageCall							};

	static bool isZeroSignature(u256 const& _r, u256 const& _s) { return !_r && !_s; }

		void clearSignature() { m_vrs = SignatureStruct(); }

	Type m_type = NullTransaction;			u256 m_nonce;							u256 m_value;							Address m_receiveAddress;				u256 m_gasPrice;						u256 m_gas;								bytes m_data;							boost::optional<SignatureStruct> m_vrs;		int m_chainId = -4;					
	mutable h256 m_hashWith;				mutable Address m_sender;			};

using TransactionBases = std::vector<TransactionBase>;

inline std::ostream& operator<<(std::ostream& _out, TransactionBase const& _t)
{
	_out << _t.sha3().abridged() << "{";
	if (_t.receiveAddress())
		_out << _t.receiveAddress().abridged();
	else
		_out << "[CREATE]";

	_out << "/" << _t.data().size() << "$" << _t.value() << "+" << _t.gas() << "@" << _t.gasPrice();
	_out << "<-" << _t.safeSender().abridged() << " #" << _t.nonce() << "}";
	return _out;
}

}
}
