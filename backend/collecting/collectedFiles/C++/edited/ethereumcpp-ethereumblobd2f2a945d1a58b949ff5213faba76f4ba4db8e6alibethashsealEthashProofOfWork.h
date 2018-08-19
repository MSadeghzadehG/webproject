


#pragma once

#include <libethcore/BlockHeader.h>
#include <libdevcore/Guards.h>

namespace dev
{
namespace eth
{

struct EthashProofOfWork
{
	struct Solution
	{
		Nonce nonce;
		h256 mixHash;
	};

	struct Result
	{
		h256 value;
		h256 mixHash;
	};

	struct WorkPackage
	{
		WorkPackage() {}
		WorkPackage(BlockHeader const& _bh);
		WorkPackage(WorkPackage const& _other);
		WorkPackage& operator=(WorkPackage const& _other);

		void reset() { Guard l(m_headerHashLock); m_headerHash = h256(); }
		operator bool() const { Guard l(m_headerHashLock); return m_headerHash != h256(); }
		h256 headerHash() const { Guard l(m_headerHashLock); return m_headerHash; }

		h256 boundary;
		h256 seedHash;

	private:
		h256 m_headerHash;			mutable Mutex m_headerHashLock;
	};

	static const WorkPackage NullWorkPackage;

		static const unsigned defaultLocalWorkSize;
		static const unsigned defaultGlobalWorkSizeMultiplier;
		static const unsigned defaultMSPerBatch;
};

}
}
