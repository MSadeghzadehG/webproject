


#pragma once

#include <string>
#include <libdevcore/CommonJS.h>
#include <libdevcrypto/Common.h>
#include "Common.h"


namespace dev
{

inline Public jsToPublic(std::string const& _s) { return jsToFixed<sizeof(dev::Public)>(_s); }

inline Secret jsToSecret(std::string const& _s) { h256 d = jsToFixed<sizeof(dev::Secret)>(_s); Secret ret(d); d.ref().cleanse(); return ret; }

inline Address jsToAddress(std::string const& _s) { return eth::toAddress(_s); }

std::string prettyU256(u256 _n, bool _abridged = true);

}


namespace dev
{
namespace eth
{

BlockNumber jsToBlockNumber(std::string const& _js);

}
}
