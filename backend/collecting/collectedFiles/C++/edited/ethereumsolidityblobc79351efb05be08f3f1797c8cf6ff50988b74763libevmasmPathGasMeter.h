


#pragma once

#include <libevmasm/GasMeter.h>

#include <libsolidity/interface/EVMVersion.h>

#include <set>
#include <vector>
#include <memory>

namespace dev
{
namespace eth
{

class KnownState;

struct GasPath
{
	size_t index = 0;
	std::shared_ptr<KnownState> state;
	u256 largestMemoryAccess;
	GasMeter::GasConsumption gas;
	std::set<size_t> visitedJumpdests;
};


class PathGasMeter
{
public:
	explicit PathGasMeter(AssemblyItems const& _items, solidity::EVMVersion _evmVersion);

	GasMeter::GasConsumption estimateMax(size_t _startIndex, std::shared_ptr<KnownState> const& _state);

private:
					void queue(std::unique_ptr<GasPath>&& _newPath);
	GasMeter::GasConsumption handleQueueItem();

			std::map<size_t, std::unique_ptr<GasPath>> m_queue;
	std::map<size_t, GasMeter::GasConsumption> m_highestGasUsagePerJumpdest;
	std::map<u256, size_t> m_tagPositions;
	AssemblyItems const& m_items;
	solidity::EVMVersion m_evmVersion;
};

}
}
