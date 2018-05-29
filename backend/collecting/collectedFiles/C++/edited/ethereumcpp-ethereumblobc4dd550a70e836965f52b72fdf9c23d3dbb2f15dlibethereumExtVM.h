

#pragma once

#include "Executive.h"
#include "State.h"

#include <libethcore/Common.h>
#include <libethcore/SealEngine.h>
#include <libevm/ExtVMFace.h>

#include <functional>
#include <map>

namespace dev
{
namespace eth
{

class SealEngineFace;

class ExtVM : public ExtVMFace
{
public:
        ExtVM(State& _s, EnvInfo const& _envInfo, SealEngineFace const& _sealEngine, Address _myAddress,
        Address _caller, Address _origin, u256 _value, u256 _gasPrice, bytesConstRef _data,
        bytesConstRef _code, h256 const& _codeHash, unsigned _depth, bool _isCreate,
        bool _staticCall)
      : ExtVMFace(_envInfo, _myAddress, _caller, _origin, _value, _gasPrice, _data, _code.toBytes(),
            _codeHash, _depth, _isCreate, _staticCall),
        m_s(_s),
        m_sealEngine(_sealEngine)
    {
                                assert(m_s.addressInUse(_myAddress));
    }

		virtual u256 store(u256 _n) override final { return m_s.storage(myAddress, _n); }

		virtual void setStore(u256 _n, u256 _v) override final;

		virtual bytes const& codeAt(Address _a) override final { return m_s.code(_a); }

		virtual size_t codeSizeAt(Address _a) override final;

		CreateResult create(u256 _endowment, u256& io_gas, bytesConstRef _code, Instruction _op, u256 _salt, OnOpFunc const& _onOp = {}) final;

		CallResult call(CallParameters& _params) final;

		virtual u256 balance(Address _a) override final { return m_s.balance(_a); }

		virtual bool exists(Address _a) override final
	{
		if (evmSchedule().emptinessIsNonexistence())
			return m_s.accountNonemptyAndExisting(_a);
		else
			return m_s.addressInUse(_a);
	}

		virtual void suicide(Address _a) override final;

		virtual EVMSchedule const& evmSchedule() const override final { return m_sealEngine.evmSchedule(envInfo().number()); }

	State const& state() const { return m_s; }

		h256 blockHash(u256 _number) override;

private:
	State& m_s;  	SealEngineFace const& m_sealEngine;
};

}
}

