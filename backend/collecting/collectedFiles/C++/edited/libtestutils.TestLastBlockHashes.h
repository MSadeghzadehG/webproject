


#pragma once

#include <libethereum/LastBlockHashesFace.h>

namespace dev
{
namespace test
{

class TestLastBlockHashes: public eth::LastBlockHashesFace
{
public:
	explicit TestLastBlockHashes(h256s const& _hashes): m_hashes(_hashes) {}

	h256s precedingHashes(h256 const& ) const override { return m_hashes; }
	void clear() override {}

private:
	h256s const m_hashes;
};


}
}