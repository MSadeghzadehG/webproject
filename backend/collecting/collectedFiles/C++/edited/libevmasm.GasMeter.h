


#pragma once

#include <libevmasm/ExpressionClasses.h>
#include <libevmasm/AssemblyItem.h>

#include <libsolidity/interface/EVMVersion.h>

#include <ostream>
#include <tuple>

namespace dev
{
namespace eth
{

class KnownState;

namespace GasCosts
{
	static unsigned const stackLimit = 1024;
	static unsigned const tier0Gas = 0;
	static unsigned const tier1Gas = 2;
	static unsigned const tier2Gas = 3;
	static unsigned const tier3Gas = 5;
	static unsigned const tier4Gas = 8;
	static unsigned const tier5Gas = 10;
	static unsigned const tier6Gas = 20;
	static unsigned const tier7Gas = 0;
	inline unsigned extCodeGas(EVMVersion _evmVersion)
	{
		return _evmVersion >= EVMVersion::tangerineWhistle() ? 700 : 20;
	}
	inline unsigned balanceGas(EVMVersion _evmVersion)
	{
		return _evmVersion >= EVMVersion::tangerineWhistle() ? 400 : 20;
	}
	static unsigned const expGas = 10;
	inline unsigned expByteGas(EVMVersion _evmVersion)
	{
		return _evmVersion >= EVMVersion::spuriousDragon() ? 50 : 10;
	}
	static unsigned const keccak256Gas = 30;
	static unsigned const keccak256WordGas = 6;
	inline unsigned sloadGas(EVMVersion _evmVersion)
	{
		return _evmVersion >= EVMVersion::tangerineWhistle() ? 200 : 50;
	}
	static unsigned const sstoreSetGas = 20000;
	static unsigned const sstoreResetGas = 5000;
	static unsigned const sstoreRefundGas = 15000;
	static unsigned const jumpdestGas = 1;
	static unsigned const logGas = 375;
	static unsigned const logDataGas = 8;
	static unsigned const logTopicGas = 375;
	static unsigned const createGas = 32000;
	inline unsigned callGas(EVMVersion _evmVersion)
	{
		return _evmVersion >= EVMVersion::tangerineWhistle() ? 700 : 40;
	}
	static unsigned const callStipend = 2300;
	static unsigned const callValueTransferGas = 9000;
	static unsigned const callNewAccountGas = 25000;
	inline unsigned selfdestructGas(EVMVersion _evmVersion)
	{
		return _evmVersion >= EVMVersion::tangerineWhistle() ? 5000 : 0;
	}
	static unsigned const selfdestructRefundGas = 24000;
	static unsigned const memoryGas = 3;
	static unsigned const quadCoeffDiv = 512;
	static unsigned const createDataGas = 200;
	static unsigned const txGas = 21000;
	static unsigned const txCreateGas = 53000;
	static unsigned const txDataZeroGas = 4;
	static unsigned const txDataNonZeroGas = 68;
	static unsigned const copyGas = 3;
}


class GasMeter
{
public:
	struct GasConsumption
	{
		GasConsumption(unsigned _value = 0, bool _infinite = false): value(_value), isInfinite(_infinite) {}
		GasConsumption(u256 _value, bool _infinite = false): value(_value), isInfinite(_infinite) {}
		static GasConsumption infinite() { return GasConsumption(0, true); }

		GasConsumption& operator+=(GasConsumption const& _other);
		bool operator<(GasConsumption const& _other) const { return this->tuple() < _other.tuple(); }

		std::tuple<bool const&, u256 const&> tuple() const { return std::tie(isInfinite, value); }

		u256 value;
		bool isInfinite;
	};

		GasMeter(std::shared_ptr<KnownState> const& _state, solidity::EVMVersion _evmVersion, u256 const& _largestMemoryAccess = 0):
		m_state(_state), m_evmVersion(_evmVersion), m_largestMemoryAccess(_largestMemoryAccess) {}

				GasConsumption estimateMax(AssemblyItem const& _item, bool _includeExternalCosts = true);

	u256 const& largestMemoryAccess() const { return m_largestMemoryAccess; }

			static unsigned runGas(Instruction _instruction);

private:
		GasConsumption wordGas(u256 const& _multiplier, ExpressionClasses::Id _value);
			GasConsumption memoryGas(ExpressionClasses::Id _position);
			GasConsumption memoryGas(int _stackPosOffset, int _stackPosSize);

	std::shared_ptr<KnownState> m_state;
	EVMVersion m_evmVersion;
		u256 m_largestMemoryAccess;
};

inline std::ostream& operator<<(std::ostream& _str, GasMeter::GasConsumption const& _consumption)
{
	if (_consumption.isInfinite)
		return _str << "[???]";
	else
		return _str << std::dec << _consumption.value;
}


}
}
