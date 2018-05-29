


#pragma once

#include <libdevcore/Common.h>
#include <libdevcore/Log.h>
#include <libdevcore/Worker.h>
#include <libethcore/Common.h>

namespace dev
{

namespace eth
{

inline std::ostream& operator<<(std::ostream& _out, WorkingProgress _p)
{
	_out << _p.rate() << " H/s = " <<  _p.hashes << " hashes / " << (double(_p.ms) / 1000) << " s";
	return _out;
}

template <class PoW> class GenericMiner;


template <class PoW> class GenericFarmFace
{
public:
	using WorkPackage = typename PoW::WorkPackage;
	using Solution = typename PoW::Solution;
	using Miner = GenericMiner<PoW>;

	virtual ~GenericFarmFace() {}

	
	virtual bool submitProof(Solution const& _p, Miner* _finder) = 0;
};


template <class PoW> class GenericMiner
{
public:
	using WorkPackage = typename PoW::WorkPackage;
	using Solution = typename PoW::Solution;
	using FarmFace = GenericFarmFace<PoW>;
	using ConstructionInfo = std::pair<FarmFace*, unsigned>;

	GenericMiner(ConstructionInfo const& _ci):
		m_farm(_ci.first),
		m_index(_ci.second)
	{}
	virtual ~GenericMiner() {}

	
	void setWork(WorkPackage const& _work = WorkPackage())
	{
		bool const old_exists = !!m_work;
		{
			Guard l(x_work);
			m_work = _work;
		}
		if (!!_work)
		{
			DEV_TIMED_ABOVE("pause", 250)
				pause();
			DEV_TIMED_ABOVE("kickOff", 250)
				kickOff();
		}
		else if (!_work && old_exists)
			pause();
		Guard l(x_hashCount);
		m_hashCount = 0;
	}

	uint64_t hashCount() const { Guard l(x_hashCount); return m_hashCount; }

	void resetHashCount() { Guard l(x_hashCount); m_hashCount = 0; }

	unsigned index() const { return m_index; }

protected:

	
	
	virtual void kickOff() = 0;

	
	virtual void pause() = 0;

	
	
	bool submitProof(Solution const& _s)
	{
		if (!m_farm)
			return true;
		if (m_farm->submitProof(_s, this))
		{
			Guard l(x_work);
			m_work.reset();
			return true;
		}
		return false;
	}

	WorkPackage const& work() const { Guard l(x_work); return m_work; }

	void accumulateHashes(unsigned _n) { Guard l(x_hashCount); m_hashCount += _n; }

private:
	FarmFace* m_farm = nullptr;
	unsigned m_index;

	uint64_t m_hashCount = 0;
	mutable Mutex x_hashCount;

	WorkPackage m_work;
	mutable Mutex x_work;
};

}
}
