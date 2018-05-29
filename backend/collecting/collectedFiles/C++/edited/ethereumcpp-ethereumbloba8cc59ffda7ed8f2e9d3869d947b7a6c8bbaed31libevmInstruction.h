

#pragma once

#include <cstdint>
#include <string>

namespace dev
{
namespace eth
{

enum class Instruction: uint8_t
{
	STOP = 0x00,        	ADD,                	MUL,                	SUB,                	DIV,                	SDIV,               	MOD,                	SMOD,               	ADDMOD,             	MULMOD,             	EXP,                	SIGNEXTEND,         
	LT = 0x10,          	GT,                 	SLT,                	SGT,                	EQ,                 	ISZERO,             	AND,                	OR,                 	XOR,                	NOT,                	BYTE,               	SHL,                	SHR,                	SAR,                
	SHA3 = 0x20,        
	ADDRESS = 0x30,     	BALANCE,            	ORIGIN,             	CALLER,             	CALLVALUE,          	CALLDATALOAD,       	CALLDATASIZE,       	CALLDATACOPY,       	CODESIZE,           	CODECOPY,           	GASPRICE,           	EXTCODESIZE,        	EXTCODECOPY,        	RETURNDATASIZE = 0x3d,  	RETURNDATACOPY = 0x3e,  
	BLOCKHASH = 0x40,   	COINBASE,           	TIMESTAMP,          	NUMBER,             	DIFFICULTY,         	GASLIMIT,           	
	POP = 0x50,         	MLOAD,              	MSTORE,             	MSTORE8,            	SLOAD,              	SSTORE,             	JUMP,               	JUMPI,              	PC,                 	MSIZE,              	GAS,                	JUMPDEST,           	
	PUSH1 = 0x60,       	PUSH2,              	PUSH3,              	PUSH4,              	PUSH5,              	PUSH6,              	PUSH7,              	PUSH8,              	PUSH9,              	PUSH10,             	PUSH11,             	PUSH12,             	PUSH13,             	PUSH14,             	PUSH15,             	PUSH16,             	PUSH17,             	PUSH18,             	PUSH19,             	PUSH20,             	PUSH21,             	PUSH22,             	PUSH23,             	PUSH24,             	PUSH25,             	PUSH26,             	PUSH27,             	PUSH28,             	PUSH29,             	PUSH30,             	PUSH31,             	PUSH32,             	
	DUP1 = 0x80,        	DUP2,               	DUP3,               	DUP4,               	DUP5,               	DUP6,               	DUP7,               	DUP8,               	DUP9,               	DUP10,              	DUP11,              	DUP12,              	DUP13,              	DUP14,              	DUP15,              	DUP16,              
	SWAP1 = 0x90,       	SWAP2,              	SWAP3,              	SWAP4,              	SWAP5,              	SWAP6,              	SWAP7,              	SWAP8,              	SWAP9,              	SWAP10,             	SWAP11,             	SWAP12,             	SWAP13,             	SWAP14,             	SWAP15,             	SWAP16,             
	LOG0 = 0xa0,        	LOG1,               	LOG2,               	LOG3,               	LOG4,               
		PUSHC = 0xac,       	JUMPC,              	JUMPCI,             
	JUMPTO = 0xb0,      	JUMPIF,             	JUMPSUB,            	JUMPV,              	JUMPSUBV,           	BEGINSUB,           	BEGINDATA,          	RETURNSUB,          	PUTLOCAL,           	GETLOCAL,           
	XADD = 0xc1,        	XMUL,               	XSUB,               	XDIV,               	XSDIV,              	XMOD,               	XSMOD,              	XLT = 0xd0,         	XGT,                	XSLT,               	XSGT,               	XEQ,                	XISZERO,            	XAND,               	XOOR,               	XXOR,               	XNOT,               	XSHL = 0xdb,        	XSHR,               	XSAR,               	XROL,               	XROR,               	XPUSH = 0xe0,       	XMLOAD,             	XMSTORE,            	XSLOAD = 0xe4,      	XSSTORE,            	XVTOWIDE,           	XWIDETOV,           	XGET,               	XPUT,               	XSWIZZLE,           	XSHUFFLE,           
	CREATE = 0xf0,      	CALL,               	CALLCODE,           	RETURN,             	DELEGATECALL,       	STATICCALL = 0xfa,		CREATE2 = 0xfb,			REVERT = 0xfd,      	INVALID = 0xfe,     	SUICIDE = 0xff      };

enum class Tier : unsigned
{
	Zero = 0,   	Base,       	VeryLow,    	Low,        	Mid,        	High,       	Ext,        	Special,    	Invalid     };

struct InstructionInfo
{
	std::string name;   	int additional;     	int args;           	int ret;            	Tier gasPriceTier;   };

struct InstructionMetric
{
	Tier gasPriceTier;
	int args;
	int ret;
};

InstructionInfo instructionInfo(Instruction _inst);

}
}
