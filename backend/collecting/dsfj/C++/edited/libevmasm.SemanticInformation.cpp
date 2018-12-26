


#include <libevmasm/SemanticInformation.h>
#include <libevmasm/AssemblyItem.h>

using namespace std;
using namespace dev;
using namespace dev::eth;

bool SemanticInformation::breaksCSEAnalysisBlock(AssemblyItem const& _item)
{
	switch (_item.type())
	{
	default:
	case UndefinedItem:
	case Tag:
	case PushDeployTimeAddress:
		return true;
	case Push:
	case PushString:
	case PushTag:
	case PushSub:
	case PushSubSize:
	case PushProgramSize:
	case PushData:
	case PushLibraryAddress:
		return false;
	case Operation:
	{
		if (isSwapInstruction(_item) || isDupInstruction(_item))
			return false;
		if (_item.instruction() == Instruction::GAS || _item.instruction() == Instruction::PC)
			return true; 		if (_item.instruction() == Instruction::MSIZE)
			return true; 		InstructionInfo info = instructionInfo(_item.instruction());
		if (_item.instruction() == Instruction::SSTORE)
			return false;
		if (_item.instruction() == Instruction::MSTORE)
			return false;
						
				return info.sideEffects || info.args > 2;
	}
	}
}

bool SemanticInformation::isCommutativeOperation(AssemblyItem const& _item)
{
	if (_item.type() != Operation)
		return false;
	switch (_item.instruction())
	{
	case Instruction::ADD:
	case Instruction::MUL:
	case Instruction::EQ:
	case Instruction::AND:
	case Instruction::OR:
	case Instruction::XOR:
		return true;
	default:
		return false;
	}
}

bool SemanticInformation::isDupInstruction(AssemblyItem const& _item)
{
	if (_item.type() != Operation)
		return false;
	return solidity::isDupInstruction(_item.instruction());
}

bool SemanticInformation::isSwapInstruction(AssemblyItem const& _item)
{
	if (_item.type() != Operation)
		return false;
	return solidity::isSwapInstruction(_item.instruction());
}

bool SemanticInformation::isJumpInstruction(AssemblyItem const& _item)
{
	return _item == AssemblyItem(Instruction::JUMP) || _item == AssemblyItem(Instruction::JUMPI);
}

bool SemanticInformation::altersControlFlow(AssemblyItem const& _item)
{
	if (_item.type() != Operation)
		return false;
	switch (_item.instruction())
	{
			case Instruction::JUMP:
	case Instruction::JUMPI:
	case Instruction::RETURN:
	case Instruction::SELFDESTRUCT:
	case Instruction::STOP:
	case Instruction::INVALID:
	case Instruction::REVERT:
		return true;
	default:
		return false;
	}
}


bool SemanticInformation::isDeterministic(AssemblyItem const& _item)
{
	if (_item.type() != Operation)
		return true;

	switch (_item.instruction())
	{
	case Instruction::CALL:
	case Instruction::CALLCODE:
	case Instruction::DELEGATECALL:
	case Instruction::STATICCALL:
	case Instruction::CREATE:
	case Instruction::CREATE2:
	case Instruction::GAS:
	case Instruction::PC:
	case Instruction::MSIZE: 	case Instruction::BALANCE: 	case Instruction::EXTCODESIZE:
	case Instruction::RETURNDATACOPY: 	case Instruction::RETURNDATASIZE:
		return false;
	default:
		return true;
	}
}

bool SemanticInformation::movable(Instruction _instruction)
{
		if (isDupInstruction(_instruction) || isSwapInstruction(_instruction))
		return false;
	InstructionInfo info = instructionInfo(_instruction);
	if (info.sideEffects)
		return false;
	switch (_instruction)
	{
	case Instruction::KECCAK256:
	case Instruction::BALANCE:
	case Instruction::EXTCODESIZE:
	case Instruction::RETURNDATASIZE:
	case Instruction::SLOAD:
	case Instruction::PC:
	case Instruction::MSIZE:
	case Instruction::GAS:
		return false;
	default:
		return true;
	}
	return true;
}

bool SemanticInformation::invalidatesMemory(Instruction _instruction)
{
	switch (_instruction)
	{
	case Instruction::CALLDATACOPY:
	case Instruction::CODECOPY:
	case Instruction::EXTCODECOPY:
	case Instruction::RETURNDATACOPY:
	case Instruction::MSTORE:
	case Instruction::MSTORE8:
	case Instruction::CALL:
	case Instruction::CALLCODE:
	case Instruction::DELEGATECALL:
	case Instruction::STATICCALL:
		return true;
	default:
		return false;
	}
}

bool SemanticInformation::invalidatesStorage(Instruction _instruction)
{
	switch (_instruction)
	{
	case Instruction::CALL:
	case Instruction::CALLCODE:
	case Instruction::DELEGATECALL:
	case Instruction::CREATE:
	case Instruction::CREATE2:
	case Instruction::SSTORE:
		return true;
	default:
		return false;
	}
}

bool SemanticInformation::invalidInPureFunctions(Instruction _instruction)
{
	switch (_instruction)
	{
	case Instruction::ADDRESS:
	case Instruction::BALANCE:
	case Instruction::ORIGIN:
	case Instruction::CALLER:
	case Instruction::CALLVALUE:
	case Instruction::GAS:
	case Instruction::GASPRICE:
	case Instruction::EXTCODESIZE:
	case Instruction::EXTCODECOPY:
	case Instruction::BLOCKHASH:
	case Instruction::COINBASE:
	case Instruction::TIMESTAMP:
	case Instruction::NUMBER:
	case Instruction::DIFFICULTY:
	case Instruction::GASLIMIT:
	case Instruction::STATICCALL:
	case Instruction::SLOAD:
		return true;
	default:
		break;
	}
	return invalidInViewFunctions(_instruction);
}

bool SemanticInformation::invalidInViewFunctions(Instruction _instruction)
{
	switch (_instruction)
	{
	case Instruction::SSTORE:
	case Instruction::JUMP:
	case Instruction::JUMPI:
	case Instruction::LOG0:
	case Instruction::LOG1:
	case Instruction::LOG2:
	case Instruction::LOG3:
	case Instruction::LOG4:
	case Instruction::CREATE:
	case Instruction::CALL:
	case Instruction::CALLCODE:
	case Instruction::DELEGATECALL:
	case Instruction::CREATE2:
	case Instruction::SELFDESTRUCT:
		return true;
	default:
		break;
	}
	return false;
}
