


#pragma once

#include <libdevcore/CommonData.h>

#include <boost/functional/hash.hpp>
#include <boost/io/ios_state.hpp>

#include <array>
#include <cstdint>
#include <algorithm>

namespace dev
{

template <unsigned N>
class FixedHash
{
public:
		using Arith = boost::multiprecision::number<boost::multiprecision::cpp_int_backend<N * 8, N * 8, boost::multiprecision::unsigned_magnitude, boost::multiprecision::unchecked, void>>;

		enum { size = N };

		enum ConstructFromStringType { FromHex, FromBinary };

		enum ConstructFromHashType { AlignLeft, AlignRight, FailIfDifferent };

		explicit FixedHash() { m_data.fill(0); }

		template <unsigned M> explicit FixedHash(FixedHash<M> const& _h, ConstructFromHashType _t = AlignLeft) { m_data.fill(0); unsigned c = std::min(M, N); for (unsigned i = 0; i < c; ++i) m_data[_t == AlignRight ? N - 1 - i : i] = _h[_t == AlignRight ? M - 1 - i : i]; }

		FixedHash(Arith const& _arith) { toBigEndian(_arith, m_data); }

		explicit FixedHash(unsigned _u) { toBigEndian(_u, m_data); }

		explicit FixedHash(bytes const& _b, ConstructFromHashType _t = FailIfDifferent) { if (_b.size() == N) memcpy(m_data.data(), _b.data(), std::min<unsigned>(_b.size(), N)); else { m_data.fill(0); if (_t != FailIfDifferent) { auto c = std::min<unsigned>(_b.size(), N); for (unsigned i = 0; i < c; ++i) m_data[_t == AlignRight ? N - 1 - i : i] = _b[_t == AlignRight ? _b.size() - 1 - i : i]; } } }

		explicit FixedHash(bytesConstRef _b, ConstructFromHashType _t = FailIfDifferent) { if (_b.size() == N) memcpy(m_data.data(), _b.data(), std::min<unsigned>(_b.size(), N)); else { m_data.fill(0); if (_t != FailIfDifferent) { auto c = std::min<unsigned>(_b.size(), N); for (unsigned i = 0; i < c; ++i) m_data[_t == AlignRight ? N - 1 - i : i] = _b[_t == AlignRight ? _b.size() - 1 - i : i]; } } }

		explicit FixedHash(std::string const& _s, ConstructFromStringType _t = FromHex, ConstructFromHashType _ht = FailIfDifferent): FixedHash(_t == FromHex ? fromHex(_s, WhenError::Throw) : dev::asBytes(_s), _ht) {}

		operator Arith() const { return fromBigEndian<Arith>(m_data); }

		explicit operator bool() const { return std::any_of(m_data.begin(), m_data.end(), [](byte _b) { return _b != 0; }); }

		bool operator==(FixedHash const& _c) const { return m_data == _c.m_data; }
	bool operator!=(FixedHash const& _c) const { return m_data != _c.m_data; }
		bool operator<(FixedHash const& _c) const { for (unsigned i = 0; i < N; ++i) if (m_data[i] < _c.m_data[i]) return true; else if (m_data[i] > _c.m_data[i]) return false; return false; }

	FixedHash operator~() const { FixedHash ret; for (unsigned i = 0; i < N; ++i) ret[i] = ~m_data[i]; return ret; }

		byte& operator[](unsigned _i) { return m_data[_i]; }
		byte operator[](unsigned _i) const { return m_data[_i]; }

		std::string hex() const { return toHex(ref()); }

		bytesRef ref() { return bytesRef(m_data.data(), N); }

		bytesConstRef ref() const { return bytesConstRef(m_data.data(), N); }

		byte* data() { return m_data.data(); }

		byte const* data() const { return m_data.data(); }

		bytes asBytes() const { return bytes(data(), data() + N); }

		std::array<byte, N>& asArray() { return m_data; }

		std::array<byte, N> const& asArray() const { return m_data; }

		inline unsigned firstBitSet() const
	{
		unsigned ret = 0;
		for (auto d: m_data)
			if (d)
			{
				for (;; ++ret, d <<= 1)
					if (d & 0x80)
						return ret;
			}
			else
				ret += 8;
		return ret;
	}

	void clear() { m_data.fill(0); }

private:
	std::array<byte, N> m_data;		};

template <unsigned N>
inline std::ostream& operator<<(std::ostream& _out, FixedHash<N> const& _h)
{
	boost::io::ios_all_saver guard(_out);
	_out << std::noshowbase << std::hex << std::setfill('0');
	for (unsigned i = 0; i < N; ++i)
		_out << std::setw(2) << (int)_h[i];
	_out << std::dec;
	return _out;
}

using h256 = FixedHash<32>;
using h160 = FixedHash<20>;

}
