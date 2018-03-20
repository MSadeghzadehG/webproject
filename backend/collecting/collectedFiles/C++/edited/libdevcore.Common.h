


#pragma once

#ifdef _M_IX86
#pragma warning(disable:4244)
#endif

#if _MSC_VER && _MSC_VER < 1900
#define _ALLOW_KEYWORD_MACROS
#define noexcept throw()
#endif

#ifdef __INTEL_COMPILER
#pragma warning(disable:3682) #endif

#include <libdevcore/vector_ref.h>

#if defined(__GNUC__)
#pragma warning(push)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif 
#include <boost/version.hpp>
#if (BOOST_VERSION == 105800)
	#include "boost_multiprecision_number_compare_bug_workaround.hpp"
#endif 
#include <boost/multiprecision/cpp_int.hpp>

#if defined(__GNUC__)
#pragma warning(pop)
#pragma GCC diagnostic pop
#endif 
#include <map>
#include <vector>
#include <functional>
#include <string>

using byte = uint8_t;

namespace dev
{

using bytes = std::vector<byte>;
using bytesRef = vector_ref<byte>;
using bytesConstRef = vector_ref<byte const>;

using bigint = boost::multiprecision::number<boost::multiprecision::cpp_int_backend<>>;
using u256 = boost::multiprecision::number<boost::multiprecision::cpp_int_backend<256, 256, boost::multiprecision::unsigned_magnitude, boost::multiprecision::unchecked, void>>;
using s256 = boost::multiprecision::number<boost::multiprecision::cpp_int_backend<256, 256, boost::multiprecision::signed_magnitude, boost::multiprecision::unchecked, void>>;
using u160 = boost::multiprecision::number<boost::multiprecision::cpp_int_backend<160, 160, boost::multiprecision::unsigned_magnitude, boost::multiprecision::unchecked, void>>;

using StringMap = std::map<std::string, std::string>;

using strings = std::vector<std::string>;

inline s256 u2s(u256 _u)
{
	static const bigint c_end = bigint(1) << 256;
	if (boost::multiprecision::bit_test(_u, 255))
		return s256(-(c_end - _u));
	else
		return s256(_u);
}

inline u256 s2u(s256 _u)
{
	static const bigint c_end = bigint(1) << 256;
    if (_u >= 0)
		return u256(_u);
    else
		return u256(c_end + _u);
}

inline std::ostream& operator<<(std::ostream& os, bytes const& _bytes)
{
	std::ostringstream ss;
	ss << std::hex;
	std::copy(_bytes.begin(), _bytes.end(), std::ostream_iterator<int>(ss, ","));
	std::string result = ss.str();
	result.pop_back();
	os << "[" + result + "]";
	return os;
}

class ScopeGuard
{
public:
	explicit ScopeGuard(std::function<void(void)> _f): m_f(_f) {}
	~ScopeGuard() { m_f(); }

private:
	std::function<void(void)> m_f;
};

}
