


#pragma once
#include <json/json.h>
#include <libdevcore/TransientDirectory.h>
#include <libethereum/BlockChain.h>
#include <libethereum/Block.h>
#include <libethashseal/GenesisInfo.h>

namespace dev
{
namespace test
{


class BlockChainLoader
{
public:
	explicit BlockChainLoader(Json::Value const& _json, eth::Network _sealEngineNetwork = eth::Network::TransitionnetTest);
	eth::BlockChain const& bc() const { return *m_bc; }
	eth::State const& state() const { return m_block.state(); }		eth::Block const& block() const { return m_block; }

private:
	TransientDirectory m_dir;
	std::unique_ptr<eth::BlockChain> m_bc;
	eth::Block m_block;
};

}
}
