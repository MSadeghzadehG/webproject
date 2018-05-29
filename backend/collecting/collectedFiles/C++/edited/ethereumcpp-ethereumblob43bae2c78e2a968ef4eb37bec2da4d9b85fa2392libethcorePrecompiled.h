


#pragma once

#include <unordered_map>
#include <functional>
#include <libdevcore/CommonData.h>
#include <libdevcore/Exceptions.h>

namespace dev
{
namespace eth
{

using PrecompiledExecutor = std::function<std::pair<bool, bytes>(bytesConstRef _in)>;
using PrecompiledPricer = std::function<bigint(bytesConstRef _in)>;

DEV_SIMPLE_EXCEPTION(ExecutorNotFound);
DEV_SIMPLE_EXCEPTION(PricerNotFound);

class PrecompiledRegistrar
{
public:
		static PrecompiledExecutor const& executor(std::string const& _name);

		static PrecompiledPricer const& pricer(std::string const& _name);

		static PrecompiledExecutor registerExecutor(std::string const& _name, PrecompiledExecutor const& _exec) { return (get()->m_execs[_name] = _exec); }
		static void unregisterExecutor(std::string const& _name) { get()->m_execs.erase(_name); }

		static PrecompiledPricer registerPricer(std::string const& _name, PrecompiledPricer const& _exec) { return (get()->m_pricers[_name] = _exec); }
		static void unregisterPricer(std::string const& _name) { get()->m_pricers.erase(_name); }

private:
	static PrecompiledRegistrar* get() { if (!s_this) s_this = new PrecompiledRegistrar; return s_this; }

	std::unordered_map<std::string, PrecompiledExecutor> m_execs;
	std::unordered_map<std::string, PrecompiledPricer> m_pricers;
	static PrecompiledRegistrar* s_this;
};

#define ETH_REGISTER_PRECOMPILED(Name) static std::pair<bool, bytes> __eth_registerPrecompiledFunction ## Name(bytesConstRef _in); static PrecompiledExecutor __eth_registerPrecompiledFactory ## Name = ::dev::eth::PrecompiledRegistrar::registerExecutor(#Name, &__eth_registerPrecompiledFunction ## Name); static std::pair<bool, bytes> __eth_registerPrecompiledFunction ## Name
#define ETH_REGISTER_PRECOMPILED_PRICER(Name) static bigint __eth_registerPricerFunction ## Name(bytesConstRef _in); static PrecompiledPricer __eth_registerPricerFactory ## Name = ::dev::eth::PrecompiledRegistrar::registerPricer(#Name, &__eth_registerPricerFunction ## Name); static bigint __eth_registerPricerFunction ## Name
}
}
