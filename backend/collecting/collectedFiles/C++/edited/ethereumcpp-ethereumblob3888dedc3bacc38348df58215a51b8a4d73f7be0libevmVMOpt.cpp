

#include "VM.h"

namespace dev
{
namespace eth
{
std::array<InstructionMetric, 256> VM::c_metrics;
void VM::initMetrics()
{
    static bool done = []() {
        for (unsigned i = 0; i < 256; ++i)
        {
            InstructionInfo op = instructionInfo((Instruction)i);
            c_metrics[i].gasPriceTier = op.gasPriceTier;
            c_metrics[i].args = op.args;
            c_metrics[i].ret = op.ret;
        }
        return true;
    }();
    (void)done;
}

void VM::copyCode(int _extraBytes)
{
                auto extendedSize = m_codeSize + _extraBytes;
    m_code.reserve(extendedSize);
    m_code.assign(m_pCode, m_pCode + m_codeSize);
    m_code.resize(extendedSize);
}

void VM::optimize()
{
    copyCode(33);

    size_t const nBytes = m_codeSize;

        
    TRACE_STR(1, "Build JUMPDEST table")
    for (size_t pc = 0; pc < nBytes; ++pc)
    {
        Instruction op = Instruction(m_code[pc]);
        TRACE_OP(2, pc, op);
                
                if (
            op == Instruction::PUSHC ||
            op == Instruction::JUMPC ||
            op == Instruction::JUMPCI
        )
        {
            TRACE_OP(1, pc, op);
            m_code[pc] = (byte)Instruction::INVALID;
        }

        if (op == Instruction::JUMPDEST)
        {
            m_jumpDests.push_back(pc);
        }
        else if (
            (byte)Instruction::PUSH1 <= (byte)op &&
            (byte)op <= (byte)Instruction::PUSH32
        )
        {
            pc += (byte)op - (byte)Instruction::PUSH1 + 1;
        }
#if EIP_615
        else if (
            op == Instruction::JUMPTO ||
            op == Instruction::JUMPIF ||
            op == Instruction::JUMPSUB)
        {
            ++pc;
            pc += 4;
        }
        else if (op == Instruction::JUMPV || op == Instruction::JUMPSUBV)
        {
            ++pc;
            pc += 4 * m_code[pc];          }
        else if (op == Instruction::BEGINSUB)
        {
            m_beginSubs.push_back(pc);
        }
        else if (op == Instruction::BEGINDATA)
        {
            break;
        }
#endif
    }
    
#ifdef EVM_DO_FIRST_PASS_OPTIMIZATION
    
    TRACE_STR(1, "Do first pass optimizations")
    for (size_t pc = 0; pc < nBytes; ++pc)
    {
        u256 val = 0;
        Instruction op = Instruction(m_code[pc]);

        if ((byte)Instruction::PUSH1 <= (byte)op && (byte)op <= (byte)Instruction::PUSH32)
        {
            byte nPush = (byte)op - (byte)Instruction::PUSH1 + 1;

                        val = m_code[pc+1];
            for (uint64_t i = pc+2, n = nPush; --n; ++i) {
                val = (val << 8) | m_code[i];
            }

        #if EVM_USE_CONSTANT_POOL

                                                if (5 < nPush)
            {
                uint16_t pool_off = m_pool.size();
                TRACE_VAL(1, "stash", val);
                TRACE_VAL(1, "... in pool at offset" , pool_off);
                m_pool.push_back(val);

                TRACE_PRE_OPT(1, pc, op);
                m_code[pc] = byte(op = Instruction::PUSHC);
                m_code[pc+3] = nPush - 2;
                m_code[pc+2] = pool_off & 0xff;
                m_code[pc+1] = pool_off >> 8;
                TRACE_POST_OPT(1, pc, op);
            }

        #endif

        #if EVM_REPLACE_CONST_JUMP    
                                                            size_t i = pc + nPush + 1;
            op = Instruction(m_code[i]);
            if (op == Instruction::JUMP)
            {
                TRACE_VAL(1, "Replace const JUMP with JUMPC to", val)
                TRACE_PRE_OPT(1, i, op);
                
                if (0 <= verifyJumpDest(val, false))
                    m_code[i] = byte(op = Instruction::JUMPC);
                
                TRACE_POST_OPT(1, i, op);
            }
            else if (op == Instruction::JUMPI)
            {
                TRACE_VAL(1, "Replace const JUMPI with JUMPCI to", val)
                TRACE_PRE_OPT(1, i, op);
                
                if (0 <= verifyJumpDest(val, false))
                    m_code[i] = byte(op = Instruction::JUMPCI);
                
                TRACE_POST_OPT(1, i, op);
            }
        #endif

            pc += nPush;
        }
    }
    TRACE_STR(1, "Finished optimizations")
#endif    
}


void VM::initEntry()
{
    m_bounce = &VM::interpretCases;     
    initMetrics();
    optimize();
}


u256 VM::exp256(u256 _base, u256 _exponent)
{
    using boost::multiprecision::limb_type;
    u256 result = 1;
    while (_exponent)
    {
        if (static_cast<limb_type>(_exponent) & 1)                result *= _base;
        _base *= _base;
        _exponent >>= 1;
    }
    return result;
}
}
}
