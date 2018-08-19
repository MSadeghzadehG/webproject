
#pragma once

namespace dev
{
namespace eth
{

#ifndef EIP_615
#define EIP_615 false
#endif

#ifndef EIP_616
#define EIP_616 false
#endif

#ifndef EVM_JUMP_DISPATCH
#ifdef __GNUC__
#define EVM_JUMP_DISPATCH true
#else
#define EVM_JUMP_DISPATCH false
#endif
#endif
#if EVM_JUMP_DISPATCH
#ifndef __GNUC__
#error "address of label extension available only on Gnu"
#endif
#else
#define EVM_SWITCH_DISPATCH true
#endif

#ifndef EVM_OPTIMIZE
#define EVM_OPTIMIZE false
#endif
#if EVM_OPTIMIZE
#define EVM_REPLACE_CONST_JUMP true
#define EVM_USE_CONSTANT_POOL true
#define EVM_DO_FIRST_PASS_OPTIMIZATION (EVM_REPLACE_CONST_JUMP || EVM_USE_CONSTANT_POOL)
#endif


#ifndef EVM_TRACE
#define EVM_TRACE 0
#endif
#if EVM_TRACE > 0

#undef ON_OP
#if EVM_TRACE > 2
#define ON_OP() \
    (cerr << "### " << ++m_nSteps << ": " << m_PC << " " << instructionInfo(m_OP).name << endl)
#else
#define ON_OP() onOperation()
#endif

#define TRACE_STR(level, str) \
    if ((level) <= EVM_TRACE) \
        cerr << "$$$ " << (str) << endl;

#define TRACE_VAL(level, name, val) \
    if ((level) <= EVM_TRACE)       \
        cerr << "=== " << (name) << " " << hex << (val) << endl;
#define TRACE_OP(level, pc, op) \
    if ((level) <= EVM_TRACE)   \
        cerr << "*** " << (pc) << " " << instructionInfo(op).name << endl;

#define TRACE_PRE_OPT(level, pc, op) \
    if ((level) <= EVM_TRACE)        \
        cerr << "<<< " << (pc) << " " << instructionInfo(op).name << endl;

#define TRACE_POST_OPT(level, pc, op) \
    if ((level) <= EVM_TRACE)         \
        cerr << ">>> " << (pc) << " " << instructionInfo(op).name << endl;
#else
#define TRACE_STR(level, str)
#define TRACE_VAL(level, name, val)
#define TRACE_OP(level, pc, op)
#define TRACE_PRE_OPT(level, pc, op)
#define TRACE_POST_OPT(level, pc, op)
#define ON_OP() onOperation()
#endif

#if 0
#define THROW_EXCEPTION(X) ((cerr << "!!! EVM EXCEPTION " << (X).what() << endl), abort())
#else
#if EVM_TRACE > 0
#define THROW_EXCEPTION(X) \
    ((cerr << "!!! EVM EXCEPTION " << (X).what() << endl), BOOST_THROW_EXCEPTION(X))
#else
#define THROW_EXCEPTION(X) BOOST_THROW_EXCEPTION(X)
#endif
#endif


#if EVM_SWITCH_DISPATCH

#define INIT_CASES
#define DO_CASES            \
    for (;;)                \
    {                       \
        fetchInstruction(); \
        switch (m_OP)       \
        {
#define CASE(name) case Instruction::name:
#define NEXT \
    ++m_PC;  \
    break;
#define CONTINUE continue;
#define BREAK return;
#define DEFAULT default:
#define WHILE_CASES \
    }               \
    }


#elif EVM_JUMP_DISPATCH

#define INIT_CASES                              \
                                                \
    static const void* const jumpTable[256] = { \
        &&STOP,                         \
        &&ADD,                                  \
        &&MUL,                                  \
        &&SUB,                                  \
        &&DIV,                                  \
        &&SDIV,                                 \
        &&MOD,                                  \
        &&SMOD,                                 \
        &&ADDMOD,                               \
        &&MULMOD,                               \
        &&EXP,                                  \
        &&SIGNEXTEND,                           \
        &&INVALID,                              \
        &&INVALID,                              \
        &&INVALID,                              \
        &&INVALID,                              \
        &&LT,                          \
        &&GT,                                   \
        &&SLT,                                  \
        &&SGT,                                  \
        &&EQ,                                   \
        &&ISZERO,                               \
        &&AND,                                  \
        &&OR,                                   \
        &&XOR,                                  \
        &&NOT,                                  \
        &&BYTE,                                 \
        &&SHL,                                  \
        &&SHR,                                  \
        &&SAR,                                  \
        &&INVALID,                              \
        &&INVALID,                              \
        &&SHA3,                        \
        &&INVALID,                              \
        &&INVALID,                              \
        &&INVALID,                              \
        &&INVALID,                              \
        &&INVALID,                              \
        &&INVALID,                              \
        &&INVALID,                              \
        &&INVALID,                              \
        &&INVALID,                              \
        &&INVALID,                              \
        &&INVALID,                              \
        &&INVALID,                              \
        &&INVALID,                              \
        &&INVALID,                              \
        &&INVALID,                              \
        &&ADDRESS,                     \
        &&BALANCE,                              \
        &&ORIGIN,                               \
        &&CALLER,                               \
        &&CALLVALUE,                            \
        &&CALLDATALOAD,                         \
        &&CALLDATASIZE,                         \
        &&CALLDATACOPY,                         \
        &&CODESIZE,                             \
        &&CODECOPY,                             \
        &&GASPRICE,                             \
        &&EXTCODESIZE,                          \
        &&EXTCODECOPY,                          \
        &&RETURNDATASIZE,                       \
        &&RETURNDATACOPY,                       \
        &&INVALID,                              \
        &&BLOCKHASH,                   \
        &&COINBASE,                             \
        &&TIMESTAMP,                            \
        &&NUMBER,                               \
        &&DIFFICULTY,                           \
        &&GASLIMIT,                             \
        &&INVALID,                              \
        &&INVALID,                              \
        &&INVALID,                              \
        &&INVALID,                              \
        &&INVALID,                              \
        &&INVALID,                              \
        &&INVALID,                              \
        &&INVALID,                              \
        &&INVALID,                              \
        &&INVALID,                              \
        &&POP,                         \
        &&MLOAD,                                \
        &&MSTORE,                               \
        &&MSTORE8,                              \
        &&SLOAD,                                \
        &&SSTORE,                               \
        &&JUMP,                                 \
        &&JUMPI,                                \
        &&PC,                                   \
        &&MSIZE,                                \
        &&GAS,                                  \
        &&JUMPDEST,                             \
        &&BEGINDATA,                            \
        &&BEGINSUB,                             \
        &&INVALID,                              \
        &&INVALID,                              \
        &&PUSH1,                       \
        &&PUSH2,                                \
        &&PUSH3,                                \
        &&PUSH4,                                \
        &&PUSH5,                                \
        &&PUSH6,                                \
        &&PUSH7,                                \
        &&PUSH8,                                \
        &&PUSH9,                                \
        &&PUSH10,                               \
        &&PUSH11,                               \
        &&PUSH12,                               \
        &&PUSH13,                               \
        &&PUSH14,                               \
        &&PUSH15,                               \
        &&PUSH16,                               \
        &&PUSH17,                      \
        &&PUSH18,                               \
        &&PUSH19,                               \
        &&PUSH20,                               \
        &&PUSH21,                               \
        &&PUSH22,                               \
        &&PUSH23,                               \
        &&PUSH24,                               \
        &&PUSH25,                               \
        &&PUSH26,                               \
        &&PUSH27,                               \
        &&PUSH28,                               \
        &&PUSH29,                               \
        &&PUSH30,                               \
        &&PUSH31,                               \
        &&PUSH32,                               \
        &&DUP1,                        \
        &&DUP2,                                 \
        &&DUP3,                                 \
        &&DUP4,                                 \
        &&DUP5,                                 \
        &&DUP6,                                 \
        &&DUP7,                                 \
        &&DUP8,                                 \
        &&DUP9,                                 \
        &&DUP10,                                \
        &&DUP11,                                \
        &&DUP12,                                \
        &&DUP13,                                \
        &&DUP14,                                \
        &&DUP15,                                \
        &&DUP16,                                \
        &&SWAP1,                       \
        &&SWAP2,                                \
        &&SWAP3,                                \
        &&SWAP4,                                \
        &&SWAP5,                                \
        &&SWAP6,                                \
        &&SWAP7,                                \
        &&SWAP8,                                \
        &&SWAP9,                                \
        &&SWAP10,                               \
        &&SWAP11,                               \
        &&SWAP12,                               \
        &&SWAP13,                               \
        &&SWAP14,                               \
        &&SWAP15,                               \
        &&SWAP16,                               \
        &&LOG0,                        \
        &&LOG1,                                 \
        &&LOG2,                                 \
        &&LOG3,                                 \
        &&LOG4,                                 \
        &&INVALID,                              \
        &&INVALID,                              \
        &&INVALID,                              \
        &&INVALID,                              \
        &&INVALID,                              \
        &&INVALID,                              \
        &&INVALID,                              \
        &&PUSHC,                                \
        &&JUMPC,                                \
        &&JUMPCI,                               \
        &&INVALID,                              \
        &&JUMPTO,                      \
        &&JUMPIF,                               \
        &&JUMPSUB,                              \
        &&JUMPV,                                \
        &&JUMPSUBV,                             \
        &&BEGINSUB,                             \
        &&BEGINDATA,                            \
        &&RETURNSUB,                            \
        &&PUTLOCAL,                             \
        &&GETLOCAL,                             \
        &&INVALID,                              \
        &&INVALID,                              \
        &&INVALID,                              \
        &&INVALID,                              \
        &&INVALID,                              \
        &&INVALID,                              \
        &&INVALID,                     \
        &&XADD,                                 \
        &&XMUL,                                 \
        &&XSUB,                                 \
        &&XDIV,                                 \
        &&XSDIV,                                \
        &&XMOD,                                 \
        &&XSMOD,                                \
        &&INVALID,                              \
        &&INVALID,                              \
        &&INVALID,                              \
        &&INVALID,                              \
        &&INVALID,                              \
        &&INVALID,                              \
        &&INVALID,                              \
        &&INVALID,                              \
        &&XLT,                          \
        &&XGT,                                  \
        &&XSLT,                                 \
        &&XSGT,                                 \
        &&XEQ,                                  \
        &&XISZERO,                              \
        &&XAND,                                 \
        &&XOOR,                                 \
        &&XXOR,                                 \
        &&XNOT,                                 \
        &&INVALID,                              \
        &&XSHL,                                 \
        &&XSHR,                                 \
        &&XSAR,                                 \
        &&XROL,                                 \
        &&XROR,                                 \
        &&XPUSH,                       \
        &&XMLOAD,                               \
        &&XMSTORE,                              \
        &&INVALID,                              \
        &&XSLOAD,                               \
        &&XSSTORE,                              \
        &&XVTOWIDE,                             \
        &&XWIDETOV,                             \
        &&XGET,                                 \
        &&XPUT,                                 \
        &&XSWIZZLE,                             \
        &&XSHUFFLE,                             \
        &&INVALID,                              \
        &&INVALID,                              \
        &&INVALID,                              \
        &&INVALID,                              \
        &&CREATE,                      \
        &&CALL,                                 \
        &&CALLCODE,                             \
        &&RETURN,                               \
        &&DELEGATECALL,                         \
        &&INVALID,                              \
        &&INVALID,                              \
        &&INVALID,                              \
        &&INVALID,                              \
        &&INVALID,                              \
        &&STATICCALL,                           \
        &&CREATE2,                              \
        &&INVALID,                              \
        &&REVERT,                               \
        &&INVALID,                              \
        &&SUICIDE,                              \
    };

#define DO_CASES        \
    fetchInstruction(); \
    goto* jumpTable[(int)m_OP];
#define CASE(name) \
    name:
#define NEXT            \
    ++m_PC;             \
    fetchInstruction(); \
    goto* jumpTable[(int)m_OP];
#define CONTINUE        \
    fetchInstruction(); \
    goto* jumpTable[(int)m_OP];
#define BREAK return;
#define DEFAULT
#define WHILE_CASES

#else
#error No opcode dispatch configured
#endif
}
}
