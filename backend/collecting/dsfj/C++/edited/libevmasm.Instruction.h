


#pragma once

#include <functional>
#include <libdevcore/Common.h>
#include <libdevcore/Assertions.h>
#include "Exceptions.h"

namespace dev
{
namespace solidity
{

DEV_SIMPLE_EXCEPTION(InvalidDeposit);
DEV_SIMPLE_EXCEPTION(InvalidOpcode);

enum class Instruction: uint8_t
{
	STOP = 0x00,			ADD,					MUL,					SUB,					DIV,					SDIV,					MOD,					SMOD,					ADDMOD,					MULMOD,					EXP,					SIGNEXTEND,			
	LT = 0x10,				GT,						SLT,					SGT,					EQ,						ISZERO,					AND,					OR,						XOR,					NOT,					BYTE,					SHL,					SHR,					SAR,				
	KECCAK256 = 0x20,		
	ADDRESS = 0x30,			BALANCE,				ORIGIN,					CALLER,					CALLVALUE,				CALLDATALOAD,			CALLDATASIZE,			CALLDATACOPY,			CODESIZE,				CODECOPY,				GASPRICE,				EXTCODESIZE,			EXTCODECOPY,			RETURNDATASIZE = 0x3d,		RETURNDATACOPY = 0x3e,	
	BLOCKHASH = 0x40,		COINBASE,				TIMESTAMP,				NUMBER,					DIFFICULTY,				GASLIMIT,			
	POP = 0x50,				MLOAD,					MSTORE,					MSTORE8,				SLOAD,					SSTORE,					JUMP,					JUMPI,					PC,						MSIZE,					GAS,					JUMPDEST,			
	PUSH1 = 0x60,			PUSH2,					PUSH3,					PUSH4,					PUSH5,					PUSH6,					PUSH7,					PUSH8,					PUSH9,					PUSH10,					PUSH11,					PUSH12,					PUSH13,					PUSH14,					PUSH15,					PUSH16,					PUSH17,					PUSH18,					PUSH19,					PUSH20,					PUSH21,					PUSH22,					PUSH23,					PUSH24,					PUSH25,					PUSH26,					PUSH27,					PUSH28,					PUSH29,					PUSH30,					PUSH31,					PUSH32,				
	DUP1 = 0x80,			DUP2,					DUP3,					DUP4,					DUP5,					DUP6,					DUP7,					DUP8,					DUP9,					DUP10,					DUP11,					DUP12,					DUP13,					DUP14,					DUP15,					DUP16,				
	SWAP1 = 0x90,			SWAP2,					SWAP3,					SWAP4,					SWAP5,					SWAP6,					SWAP7,					SWAP8,					SWAP9,					SWAP10,					SWAP11,					SWAP12,					SWAP13,					SWAP14,					SWAP15,					SWAP16,				
	LOG0 = 0xa0,			LOG1,					LOG2,					LOG3,					LOG4,				
	JUMPTO = 0xb0,      	JUMPIF,             	JUMPV,              	JUMPSUB,            	JUMPSUBV,           	BEGINSUB,           	BEGINDATA,          	RETURNSUB,          	PUTLOCAL,           	GETLOCAL,           
	CREATE = 0xf0,			CALL,					CALLCODE,				RETURN,					DELEGATECALL,			STATICCALL = 0xfa,		CREATE2 = 0xfb,		
	REVERT = 0xfd,			INVALID = 0xfe,			SELFDESTRUCT = 0xff	};

inline bool isPushInstruction(Instruction _inst)
{
	return Instruction::PUSH1 <= _inst && _inst <= Instruction::PUSH32;
}

inline bool isDupInstruction(Instruction _inst)
{
	return Instruction::DUP1 <= _inst && _inst <= Instruction::DUP16;
}

inline bool isSwapInstruction(Instruction _inst)
{
	return Instruction::SWAP1 <= _inst && _inst <= Instruction::SWAP16;
}

inline unsigned getPushNumber(Instruction _inst)
{
	return (byte)_inst - unsigned(Instruction::PUSH1) + 1;
}

inline unsigned getDupNumber(Instruction _inst)
{
	return (byte)_inst - unsigned(Instruction::DUP1) + 1;
}

inline unsigned getSwapNumber(Instruction _inst)
{
	return (byte)_inst - unsigned(Instruction::SWAP1) + 1;
}

inline Instruction pushInstruction(unsigned _number)
{
	assertThrow(1 <= _number && _number <= 32, InvalidOpcode, std::string("Invalid PUSH instruction requested (") + std::to_string(_number) + ").");
	return Instruction(unsigned(Instruction::PUSH1) + _number - 1);
}

inline Instruction dupInstruction(unsigned _number)
{
	assertThrow(1 <= _number && _number <= 16, InvalidOpcode, std::string("Invalid DUP instruction requested (") + std::to_string(_number) + ").");
	return Instruction(unsigned(Instruction::DUP1) + _number - 1);
}

inline Instruction swapInstruction(unsigned _number)
{
	assertThrow(1 <= _number && _number <= 16, InvalidOpcode, std::string("Invalid SWAP instruction requested (") + std::to_string(_number) + ").");
	return Instruction(unsigned(Instruction::SWAP1) + _number - 1);
}

inline Instruction logInstruction(unsigned _number)
{
	assertThrow(_number <= 4, InvalidOpcode, std::string("Invalid LOG instruction requested (") + std::to_string(_number) + ").");
	return Instruction(unsigned(Instruction::LOG0) + _number);
}

enum class Tier : unsigned
{
	Zero = 0,		Base,			VeryLow,		Low,			Mid,			High,			Ext,			ExtCode,		Balance,		Special,		Invalid		};

struct InstructionInfo
{
	std::string name;		int additional;			int args;				int ret;				bool sideEffects;		Tier gasPriceTier;	};

InstructionInfo instructionInfo(Instruction _inst);

bool isValidInstruction(Instruction _inst);

extern const std::map<std::string, Instruction> c_instructions;

void eachInstruction(bytes const& _mem, std::function<void(Instruction,u256 const&)> const& _onInstruction);

std::string disassemble(bytes const& _mem);

}
}
