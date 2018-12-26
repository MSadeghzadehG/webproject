
#include "SPVRemapper.h"
#include "doc.h"

#if !defined (use_cpp11)
#else 
#include <algorithm>
#include <cassert>
#include "../glslang/Include/Common.h"

namespace spv {

        spirvbin_t::errorfn_t spirvbin_t::errorHandler = [](const std::string&) { exit(5); };
        spirvbin_t::logfn_t   spirvbin_t::logHandler   = [](const std::string&) { };

        void spirvbin_t::msg(int minVerbosity, int indent, const std::string& txt) const
    {
        if (verbose >= minVerbosity)
            logHandler(std::string(indent, ' ') + txt);
    }

        std::uint32_t spirvbin_t::asOpCodeHash(unsigned word)
    {
        const spv::Op opCode = asOpCode(word);

        std::uint32_t offset = 0;

        switch (opCode) {
        case spv::OpExtInst:
            offset += asId(word + 4); break;
        default:
            break;
        }

        return opCode * 19 + offset;     }

    spirvbin_t::range_t spirvbin_t::literalRange(spv::Op opCode) const
    {
        static const int maxCount = 1<<30;

        switch (opCode) {
        case spv::OpTypeFloat:                case spv::OpTypePointer:      return range_t(2, 3);
        case spv::OpTypeInt:          return range_t(2, 4);
                        case spv::OpTypeSampler:      return range_t(3, 8);
        case spv::OpTypeVector:               case spv::OpTypeMatrix:               case spv::OpTypePipe:         return range_t(3, 4);
        case spv::OpConstant:         return range_t(3, maxCount);
        default:                      return range_t(0, 0);
        }
    }

    spirvbin_t::range_t spirvbin_t::typeRange(spv::Op opCode) const
    {
        static const int maxCount = 1<<30;

        if (isConstOp(opCode))
            return range_t(1, 2);

        switch (opCode) {
        case spv::OpTypeVector:               case spv::OpTypeMatrix:               case spv::OpTypeSampler:              case spv::OpTypeArray:                case spv::OpTypeRuntimeArray:         case spv::OpTypePipe:         return range_t(2, 3);
        case spv::OpTypeStruct:               case spv::OpTypeFunction:     return range_t(2, maxCount);
        case spv::OpTypePointer:      return range_t(3, 4);
        default:                      return range_t(0, 0);
        }
    }

    spirvbin_t::range_t spirvbin_t::constRange(spv::Op opCode) const
    {
        static const int maxCount = 1<<30;

        switch (opCode) {
        case spv::OpTypeArray:                 case spv::OpTypeRuntimeArray:  return range_t(3, 4);
        case spv::OpConstantComposite: return range_t(3, maxCount);
        default:                       return range_t(0, 0);
        }
    }

        bool spirvbin_t::isStripOp(spv::Op opCode) const
    {
        switch (opCode) {
        case spv::OpSource:
        case spv::OpSourceExtension:
        case spv::OpName:
        case spv::OpMemberName:
        case spv::OpLine:           return true;
        default:                    return false;
        }
    }

    bool spirvbin_t::isFlowCtrl(spv::Op opCode) const
    {
        switch (opCode) {
        case spv::OpBranchConditional:
        case spv::OpBranch:
        case spv::OpSwitch:
        case spv::OpLoopMerge:
        case spv::OpSelectionMerge:
        case spv::OpLabel:
        case spv::OpFunction:
        case spv::OpFunctionEnd:    return true;
        default:                    return false;
        }
    }

    bool spirvbin_t::isTypeOp(spv::Op opCode) const
    {
        switch (opCode) {
        case spv::OpTypeVoid:
        case spv::OpTypeBool:
        case spv::OpTypeInt:
        case spv::OpTypeFloat:
        case spv::OpTypeVector:
        case spv::OpTypeMatrix:
        case spv::OpTypeImage:
        case spv::OpTypeSampler:
        case spv::OpTypeArray:
        case spv::OpTypeRuntimeArray:
        case spv::OpTypeStruct:
        case spv::OpTypeOpaque:
        case spv::OpTypePointer:
        case spv::OpTypeFunction:
        case spv::OpTypeEvent:
        case spv::OpTypeDeviceEvent:
        case spv::OpTypeReserveId:
        case spv::OpTypeQueue:
        case spv::OpTypeSampledImage:
        case spv::OpTypePipe:         return true;
        default:                      return false;
        }
    }

    bool spirvbin_t::isConstOp(spv::Op opCode) const
    {
        switch (opCode) {
        case spv::OpConstantNull:       error("unimplemented constant type");
        case spv::OpConstantSampler:    error("unimplemented constant type");

        case spv::OpConstantTrue:
        case spv::OpConstantFalse:
        case spv::OpConstantComposite:
        case spv::OpConstant:         return true;
        default:                      return false;
        }
    }

    const auto inst_fn_nop = [](spv::Op, unsigned) { return false; };
    const auto op_fn_nop   = [](spv::Id&)          { };

                const spv::Id spirvbin_t::unmapped    = spv::Id(-10000);
    const spv::Id spirvbin_t::unused      = spv::Id(-10001);
    const int     spirvbin_t::header_size = 5;

    spv::Id spirvbin_t::nextUnusedId(spv::Id id)
    {
        while (isNewIdMapped(id))              ++id;

        return id;
    }

    spv::Id spirvbin_t::localId(spv::Id id, spv::Id newId)
    {
        assert(id != spv::NoResult && newId != spv::NoResult);

        if (id >= idMapL.size())
            idMapL.resize(id+1, unused);

        if (newId != unmapped && newId != unused) {
            if (isOldIdUnused(id))
                error(std::string("ID unused in module: ") + std::to_string(id));

            if (!isOldIdUnmapped(id))
                error(std::string("ID already mapped: ") + std::to_string(id) + " -> "
                + std::to_string(localId(id)));

            if (isNewIdMapped(newId))
                error(std::string("ID already used in module: ") + std::to_string(newId));

            msg(4, 4, std::string("map: ") + std::to_string(id) + " -> " + std::to_string(newId));
            setMapped(newId);
            largestNewId = std::max(largestNewId, newId);
        }

        return idMapL[id] = newId;
    }

            std::string spirvbin_t::literalString(unsigned word) const
    {
        std::string literal;

        literal.reserve(16);

        const char* bytes = reinterpret_cast<const char*>(spv.data() + word);

        while (bytes && *bytes)
            literal += *bytes++;

        return literal;
    }


    void spirvbin_t::applyMap()
    {
        msg(3, 2, std::string("Applying map: "));

                process(inst_fn_nop,             [this](spv::Id& id) {
                id = localId(id);
                assert(id != unused && id != unmapped);
            }
        );
    }


        void spirvbin_t::mapRemainder()
    {
        msg(3, 2, std::string("Remapping remainder: "));

        spv::Id     unusedId  = 1;          spirword_t  maxBound  = 0;

        for (spv::Id id = 0; id < idMapL.size(); ++id) {
            if (isOldIdUnused(id))
                continue;

                        if (isOldIdUnmapped(id))
                localId(id, unusedId = nextUnusedId(unusedId));

            if (isOldIdUnmapped(id))
                error(std::string("old ID not mapped: ") + std::to_string(id));

                        maxBound = std::max(maxBound, localId(id) + 1);
        }

        bound(maxBound);     }

    void spirvbin_t::stripDebug()
    {
        if ((options & STRIP) == 0)
            return;

                process(
            [&](spv::Op opCode, unsigned start) {
                                if (isStripOp(opCode))
                    stripInst(start);
                return true;
            },
            op_fn_nop);
    }

    void spirvbin_t::buildLocalMaps()
    {
        msg(2, 2, std::string("build local maps: "));

        mapped.clear();
        idMapL.clear();
        fnPos.clear();
        fnPosDCE.clear();
        fnCalls.clear();
        typeConstPos.clear();
        typeConstPosR.clear();
        entryPoint = spv::NoResult;
        largestNewId = 0;

        idMapL.resize(bound(), unused);

        int         fnStart = 0;
        spv::Id     fnRes   = spv::NoResult;

                process(
            [&](spv::Op opCode, unsigned start) {
                                if ((options & STRIP) && isStripOp(opCode))
                    stripInst(start);

                if (opCode == spv::Op::OpName) {
                    const spv::Id    target = asId(start+1);
                    const std::string  name = literalString(start+2);
                    nameMap[name] = target;

                } else if (opCode == spv::Op::OpFunctionCall) {
                    ++fnCalls[asId(start + 3)];
                } else if (opCode == spv::Op::OpEntryPoint) {
                    entryPoint = asId(start + 2);
                } else if (opCode == spv::Op::OpFunction) {
                    if (fnStart != 0)
                        error("nested function found");
                    fnStart = start;
                    fnRes   = asId(start + 2);
                } else if (opCode == spv::Op::OpFunctionEnd) {
                    assert(fnRes != spv::NoResult);
                    if (fnStart == 0)
                        error("function end without function start");
                    fnPos[fnRes] = range_t(fnStart, start + asWordCount(start));
                    fnStart = 0;
                } else if (isConstOp(opCode)) {
                    assert(asId(start + 2) != spv::NoResult);
                    typeConstPos.insert(start);
                    typeConstPosR[asId(start + 2)] = start;
                } else if (isTypeOp(opCode)) {
                    assert(asId(start + 1) != spv::NoResult);
                    typeConstPos.insert(start);
                    typeConstPosR[asId(start + 1)] = start;
                }

                return false;
            },

            [this](spv::Id& id) { localId(id, unmapped); }
        );
    }

        void spirvbin_t::validate() const
    {
        msg(2, 2, std::string("validating: "));

        if (spv.size() < header_size)
            error("file too short: ");

        if (magic() != spv::MagicNumber)
            error("bad magic number");

                        
        if (schemaNum() != 0)
            error("bad schema, must be 0");
    }


    int spirvbin_t::processInstruction(unsigned word, instfn_t instFn, idfn_t idFn)
    {
        const auto     instructionStart = word;
        const unsigned wordCount = asWordCount(instructionStart);
        const spv::Op  opCode    = asOpCode(instructionStart);
        const int      nextInst  = word++ + wordCount;

        if (nextInst > int(spv.size()))
            error("spir instruction terminated too early");

                unsigned numOperands = wordCount - 1;

        if (instFn(opCode, instructionStart))
            return nextInst;

                if (spv::InstructionDesc[opCode].hasType()) {
            idFn(asId(word++));
            --numOperands;
        }

        if (spv::InstructionDesc[opCode].hasResult()) {
            idFn(asId(word++));
            --numOperands;
        }

                        if (opCode == spv::OpExtInst) {
            word        += 2;             numOperands -= 2;

            for (unsigned op=0; op < numOperands; ++op)
                idFn(asId(word++)); 
            return nextInst;
        }

                for (int op = 0; numOperands > 0; ++op, --numOperands) {
            switch (spv::InstructionDesc[opCode].operands.getClass(op)) {
            case spv::OperandId:
                idFn(asId(word++));
                break;

            case spv::OperandVariableIds:
                for (unsigned i = 0; i < numOperands; ++i)
                    idFn(asId(word++));
                return nextInst;

            case spv::OperandVariableLiterals:
                                                                                                                return nextInst;

            case spv::OperandVariableLiteralId:
                while (numOperands > 0) {
                    ++word;                                 idFn(asId(word++));                     numOperands -= 2;
                }
                return nextInst;

            case spv::OperandLiteralString: {
                const int stringWordCount = literalStringWords(literalString(word));
                word += stringWordCount;
                numOperands -= (stringWordCount-1);                 break;
            }

                        case spv::OperandExecutionMode:
                return nextInst;

                        case spv::OperandLiteralNumber:
            case spv::OperandSource:
            case spv::OperandExecutionModel:
            case spv::OperandAddressing:
            case spv::OperandMemory:
            case spv::OperandStorage:
            case spv::OperandDimensionality:
            case spv::OperandSamplerAddressingMode:
            case spv::OperandSamplerFilterMode:
            case spv::OperandSamplerImageFormat:
            case spv::OperandImageChannelOrder:
            case spv::OperandImageChannelDataType:
            case spv::OperandImageOperands:
            case spv::OperandFPFastMath:
            case spv::OperandFPRoundingMode:
            case spv::OperandLinkageType:
            case spv::OperandAccessQualifier:
            case spv::OperandFuncParamAttr:
            case spv::OperandDecoration:
            case spv::OperandBuiltIn:
            case spv::OperandSelect:
            case spv::OperandLoop:
            case spv::OperandFunction:
            case spv::OperandMemorySemantics:
            case spv::OperandMemoryAccess:
            case spv::OperandScope:
            case spv::OperandGroupOperation:
            case spv::OperandKernelEnqueueFlags:
            case spv::OperandKernelProfilingInfo:
            case spv::OperandCapability:
                ++word;
                break;

            default:
                assert(0 && "Unhandled Operand Class");
                break;
            }
        }

        return nextInst;
    }

        spirvbin_t& spirvbin_t::process(instfn_t instFn, idfn_t idFn, unsigned begin, unsigned end)
    {
                nameMap.reserve(32);

                begin = (begin == 0 ? header_size          : begin);
        end   = (end   == 0 ? unsigned(spv.size()) : end);

                unsigned nextInst = unsigned(spv.size());

        for (unsigned word = begin; word < end; word = nextInst)
            nextInst = processInstruction(word, instFn, idFn);

        return *this;
    }

        void spirvbin_t::mapNames()
    {
        static const std::uint32_t softTypeIdLimit = 3011;          static const std::uint32_t firstMappedID   = 3019;  
        for (const auto& name : nameMap) {
            std::uint32_t hashval = 1911;
            for (const char c : name.first)
                hashval = hashval * 1009 + c;

            if (isOldIdUnmapped(name.second))
                localId(name.second, nextUnusedId(hashval % softTypeIdLimit + firstMappedID));
        }
    }

        void spirvbin_t::mapFnBodies()
    {
        static const std::uint32_t softTypeIdLimit = 19071;          static const std::uint32_t firstMappedID   =  6203;  
                
        spv::Id               fnId       = spv::NoResult;
        std::vector<unsigned> instPos;
        instPos.reserve(unsigned(spv.size()) / 16); 
                process(
            [&](spv::Op, unsigned start) { instPos.push_back(start); return true; },
            op_fn_nop);

                                                static const unsigned windowSize = 2;

        for (unsigned entry = 0; entry < unsigned(instPos.size()); ++entry) {
            const unsigned start  = instPos[entry];
            const spv::Op  opCode = asOpCode(start);

            if (opCode == spv::OpFunction)
                fnId   = asId(start + 2);

            if (opCode == spv::OpFunctionEnd)
                fnId = spv::NoResult;

            if (fnId != spv::NoResult) {                 if (spv::InstructionDesc[opCode].hasResult()) {
                    const unsigned word    = start + (spv::InstructionDesc[opCode].hasType() ? 2 : 1);
                    const spv::Id  resId   = asId(word);
                    std::uint32_t  hashval = fnId * 17; 
                    for (unsigned i = entry-1; i >= entry-windowSize; --i) {
                        if (asOpCode(instPos[i]) == spv::OpFunction)
                            break;
                        hashval = hashval * 30103 + asOpCodeHash(instPos[i]);                     }

                    for (unsigned i = entry; i <= entry + windowSize; ++i) {
                        if (asOpCode(instPos[i]) == spv::OpFunctionEnd)
                            break;
                        hashval = hashval * 30103 + asOpCodeHash(instPos[i]);                     }

                    if (isOldIdUnmapped(resId))
                        localId(resId, nextUnusedId(hashval % softTypeIdLimit + firstMappedID));
                }
            }
        }

        spv::Op          thisOpCode(spv::OpNop);
        std::unordered_map<int, int> opCounter;
        int              idCounter(0);
        fnId = spv::NoResult;

        process(
            [&](spv::Op opCode, unsigned start) {
                switch (opCode) {
                case spv::OpFunction:
                                        idCounter = 0;
                    opCounter.clear();
                    fnId = asId(start + 2);
                    break;

                case spv::OpImageSampleImplicitLod:
                case spv::OpImageSampleExplicitLod:
                case spv::OpImageSampleDrefImplicitLod:
                case spv::OpImageSampleDrefExplicitLod:
                case spv::OpImageSampleProjImplicitLod:
                case spv::OpImageSampleProjExplicitLod:
                case spv::OpImageSampleProjDrefImplicitLod:
                case spv::OpImageSampleProjDrefExplicitLod:
                case spv::OpDot:
                case spv::OpCompositeExtract:
                case spv::OpCompositeInsert:
                case spv::OpVectorShuffle:
                case spv::OpLabel:
                case spv::OpVariable:

                case spv::OpAccessChain:
                case spv::OpLoad:
                case spv::OpStore:
                case spv::OpCompositeConstruct:
                case spv::OpFunctionCall:
                    ++opCounter[opCode];
                    idCounter = 0;
                    thisOpCode = opCode;
                    break;
                default:
                    thisOpCode = spv::OpNop;
                }

                return false;
            },

            [&](spv::Id& id) {
                if (thisOpCode != spv::OpNop) {
                    ++idCounter;
                    const std::uint32_t hashval = opCounter[thisOpCode] * thisOpCode * 50047 + idCounter + fnId * 117;

                    if (isOldIdUnmapped(id))
                        localId(id, nextUnusedId(hashval % softTypeIdLimit + firstMappedID));
                }
            });
    }

            void spirvbin_t::forwardLoadStores()
    {
        idset_t fnLocalVars;         idmap_t idMap;       
                process(
            [&](spv::Op opCode, unsigned start) {
                                if ((opCode == spv::OpVariable && asWordCount(start) == 4) &&
                    (spv[start+3] == spv::StorageClassUniform ||
                    spv[start+3] == spv::StorageClassUniformConstant ||
                    spv[start+3] == spv::StorageClassInput))
                    fnLocalVars.insert(asId(start+2));

                if (opCode == spv::OpAccessChain && fnLocalVars.count(asId(start+3)) > 0)
                    fnLocalVars.insert(asId(start+2));

                if (opCode == spv::OpLoad && fnLocalVars.count(asId(start+3)) > 0) {
                    idMap[asId(start+2)] = asId(start+3);
                    stripInst(start);
                }

                return false;
            },

            [&](spv::Id& id) { if (idMap.find(id) != idMap.end()) id = idMap[id]; }
        );

                fnLocalVars.clear();
        idMap.clear();

        process(
            [&](spv::Op opCode, unsigned start) {
                                if ((opCode == spv::OpVariable && asWordCount(start) == 4) &&
                    (spv[start+3] == spv::StorageClassOutput))
                    fnLocalVars.insert(asId(start+2));

                if (opCode == spv::OpStore && fnLocalVars.count(asId(start+1)) > 0) {
                    idMap[asId(start+2)] = asId(start+1);
                    stripInst(start);
                }

                return false;
            },
            op_fn_nop);

        process(
            inst_fn_nop,
            [&](spv::Id& id) { if (idMap.find(id) != idMap.end()) id = idMap[id]; }
        );

        strip();              }

        void spirvbin_t::optLoadStore()
    {
        idset_t    fnLocalVars;          idmap_t    idMap;                blockmap_t blockMap;             int        blockNum = 0; 
                process(
            [&](spv::Op opCode, unsigned start) {
                const int wordCount = asWordCount(start);

                                if (isFlowCtrl(opCode))
                    ++blockNum;

                                if ((opCode == spv::OpVariable && spv[start+3] == spv::StorageClassFunction && asWordCount(start) == 4)) {
                    fnLocalVars.insert(asId(start+2));
                    return true;
                }

                                if ((opCode == spv::OpAccessChain || opCode == spv::OpInBoundsAccessChain) && fnLocalVars.count(asId(start+3)) > 0) {
                    fnLocalVars.erase(asId(start+3));
                    idMap.erase(asId(start+3));
                    return true;
                }

                if (opCode == spv::OpLoad && fnLocalVars.count(asId(start+3)) > 0) {
                    const spv::Id varId = asId(start+3);

                                        if (idMap.find(varId) == idMap.end()) {
                        fnLocalVars.erase(varId);
                        idMap.erase(varId);
                    }

                                        if (wordCount > 4 && (spv[start+4] & spv::MemoryAccessVolatileMask)) {
                        fnLocalVars.erase(varId);
                        idMap.erase(varId);
                    }

                                        if (blockMap.find(varId) == blockMap.end()) {
                        blockMap[varId] = blockNum;                      } else if (blockMap[varId] != blockNum) {
                        fnLocalVars.erase(varId);                          idMap.erase(varId);
                    }

                    return true;
                }

                if (opCode == spv::OpStore && fnLocalVars.count(asId(start+1)) > 0) {
                    const spv::Id varId = asId(start+1);

                    if (idMap.find(varId) == idMap.end()) {
                        idMap[varId] = asId(start+2);
                    } else {
                                                fnLocalVars.erase(varId);
                        idMap.erase(varId);
                    }

                                        if (wordCount > 3 && (spv[start+3] & spv::MemoryAccessVolatileMask)) {
                        fnLocalVars.erase(asId(start+3));
                        idMap.erase(asId(start+3));
                    }

                                        if (blockMap.find(varId) == blockMap.end()) {
                        blockMap[varId] = blockNum;                      } else if (blockMap[varId] != blockNum) {
                        fnLocalVars.erase(varId);                          idMap.erase(varId);
                    }

                    return true;
                }

                return false;
            },

                        [&](spv::Id& id) { 
                if (fnLocalVars.count(id) > 0) {
                    fnLocalVars.erase(id);
                    idMap.erase(id);
                }
            }
        );

        process(
            [&](spv::Op opCode, unsigned start) {
                if (opCode == spv::OpLoad && fnLocalVars.count(asId(start+3)) > 0)
                    idMap[asId(start+2)] = idMap[asId(start+3)];
                return false;
            },
            op_fn_nop);

                                                        for (const auto& idPair : idMap) {
            spv::Id id = idPair.first;
            while (idMap.find(id) != idMap.end())                  id = idMap[id];

            idMap[idPair.first] = id;                      }

                process(
            [&](spv::Op opCode, unsigned start) {
                if ((opCode == spv::OpLoad  && fnLocalVars.count(asId(start+3)) > 0) ||
                    (opCode == spv::OpStore && fnLocalVars.count(asId(start+1)) > 0) ||
                    (opCode == spv::OpVariable && fnLocalVars.count(asId(start+2)) > 0)) {

                    stripInst(start);
                    return true;
                }

                return false;
            },

            [&](spv::Id& id) {
                if (idMap.find(id) != idMap.end()) id = idMap[id];
            }
        );

        strip();              }

        void spirvbin_t::dceFuncs()
    {
        msg(3, 2, std::string("Removing Dead Functions: "));

                bool changed = true;

        while (changed) {
            changed = false;

            for (auto fn = fnPos.begin(); fn != fnPos.end(); ) {
                if (fn->first == entryPoint) {                     ++fn;
                    continue;
                }

                const auto call_it = fnCalls.find(fn->first);

                if (call_it == fnCalls.end() || call_it->second == 0) {
                    changed = true;
                    stripRange.push_back(fn->second);
                    fnPosDCE.insert(*fn);

                                        process(
                        [&](spv::Op opCode, unsigned start) {
                            if (opCode == spv::Op::OpFunctionCall) {
                                const auto call_it = fnCalls.find(asId(start + 3));
                                if (call_it != fnCalls.end()) {
                                    if (--call_it->second <= 0)
                                        fnCalls.erase(call_it);
                                }
                            }

                            return true;
                        },
                        op_fn_nop,
                        fn->second.first,
                        fn->second.second);

                    fn = fnPos.erase(fn);
                } else ++fn;
            }
        }
    }

        void spirvbin_t::dceVars()
    {
        msg(3, 2, std::string("DCE Vars: "));

        std::unordered_map<spv::Id, int> varUseCount;

                process(
            [&](spv::Op opCode, unsigned start) {
                if (opCode == spv::OpVariable) {
                    ++varUseCount[asId(start+2)];
                    return true;
                } else if (opCode == spv::OpEntryPoint) {
                    const int wordCount = asWordCount(start);
                    for (int i = 4; i < wordCount; i++) {
                        ++varUseCount[asId(start+i)];
                    }
                    return true;
                } else
                    return false;
            },

            [&](spv::Id& id) { if (varUseCount[id]) ++varUseCount[id]; }
        );

                process(
            [&](spv::Op opCode, unsigned start) {
                if ((opCode == spv::OpVariable && varUseCount[asId(start+2)] == 1)  ||
                    (opCode == spv::OpDecorate && varUseCount[asId(start+1)] == 1)  ||
                    (opCode == spv::OpName     && varUseCount[asId(start+1)] == 1)) {
                        stripInst(start);
                }
                return true;
            },
            op_fn_nop);
    }

        void spirvbin_t::dceTypes()
    {
        std::vector<bool> isType(bound(), false);

                for (const auto typeStart : typeConstPos)
            isType[asTypeConstId(typeStart)] = true;

        std::unordered_map<spv::Id, int> typeUseCount;

                process(inst_fn_nop,
            [&](spv::Id& id) { if (isType[id]) ++typeUseCount[id]; }
        );

                for (const auto& fn : fnPosDCE)
            process(inst_fn_nop,
            [&](spv::Id& id) { if (isType[id]) --typeUseCount[id]; },
            fn.second.first, fn.second.second);

                for (const auto typeStart : typeConstPos) {
            const spv::Id typeId = asTypeConstId(typeStart);
            if (typeUseCount[typeId] == 1) {
                --typeUseCount[typeId];
                stripInst(typeStart);
            }
        }
    }


#ifdef NOTDEF
    bool spirvbin_t::matchType(const spirvbin_t::globaltypes_t& globalTypes, spv::Id lt, spv::Id gt) const
    {
                const auto lt_it = typeConstPosR.find(lt);
        if (lt_it == typeConstPosR.end())
            return false;

        const auto typeStart = lt_it->second;

                const auto gtype = globalTypes.find(gt);
        if (gtype == globalTypes.end())
            return false;

        const auto& gdata = gtype->second;

                const int     wordCount   = asWordCount(typeStart);
        const spv::Op opCode      = asOpCode(typeStart);

                if (opCode != opOpCode(gdata[0]) || wordCount != opWordCount(gdata[0]))
            return false;

        const unsigned numOperands = wordCount - 2; 
        const auto cmpIdRange = [&](range_t range) {
            for (int x=range.first; x<std::min(range.second, wordCount); ++x)
                if (!matchType(globalTypes, asId(typeStart+x), gdata[x]))
                    return false;
            return true;
        };

        const auto cmpConst   = [&]() { return cmpIdRange(constRange(opCode)); };
        const auto cmpSubType = [&]() { return cmpIdRange(typeRange(opCode));  };

                const auto cmpLiteral = [&]() {
            const auto range = literalRange(opCode);
            return std::equal(spir.begin() + typeStart + range.first,
                spir.begin() + typeStart + std::min(range.second, wordCount),
                gdata.begin() + range.first);
        };

        assert(isTypeOp(opCode) || isConstOp(opCode));

        switch (opCode) {
        case spv::OpTypeOpaque:               case spv::OpTypeQueue:        return false;
        case spv::OpTypeEvent:                case spv::OpTypeDeviceEvent:          case spv::OpTypeReserveId:    return false;
                    case spv::OpTypeSampler:      return cmpLiteral() && cmpConst() && cmpSubType() && wordCount == 8;
        default:                      return cmpLiteral() && cmpConst() && cmpSubType();
        }
    }


        spv::Id spirvbin_t::findType(const spirvbin_t::globaltypes_t& globalTypes, spv::Id lt) const
    {
                for (const auto& gt : globalTypes)
            if (matchType(globalTypes, lt, gt.first))
                return gt.first;

        return spv::NoType;
    }
#endif 
        unsigned spirvbin_t::typePos(spv::Id id) const
    {
        const auto tid_it = typeConstPosR.find(id);
        if (tid_it == typeConstPosR.end())
            error("type ID not found");

        return tid_it->second;
    }

            std::uint32_t spirvbin_t::hashType(unsigned typeStart) const
    {
        const unsigned wordCount   = asWordCount(typeStart);
        const spv::Op  opCode      = asOpCode(typeStart);

        switch (opCode) {
        case spv::OpTypeVoid:         return 0;
        case spv::OpTypeBool:         return 1;
        case spv::OpTypeInt:          return 3 + (spv[typeStart+3]);
        case spv::OpTypeFloat:        return 5;
        case spv::OpTypeVector:
            return 6 + hashType(typePos(spv[typeStart+2])) * (spv[typeStart+3] - 1);
        case spv::OpTypeMatrix:
            return 30 + hashType(typePos(spv[typeStart+2])) * (spv[typeStart+3] - 1);
        case spv::OpTypeImage:
            return 120 + hashType(typePos(spv[typeStart+2])) +
                spv[typeStart+3] +                            spv[typeStart+4] * 8 * 16 +                   spv[typeStart+5] * 4 * 16 +                   spv[typeStart+6] * 2 * 16 +                   spv[typeStart+7] * 1 * 16;            case spv::OpTypeSampler:
            return 500;
        case spv::OpTypeSampledImage:
            return 502;
        case spv::OpTypeArray:
            return 501 + hashType(typePos(spv[typeStart+2])) * spv[typeStart+3];
        case spv::OpTypeRuntimeArray:
            return 5000  + hashType(typePos(spv[typeStart+2]));
        case spv::OpTypeStruct:
            {
                std::uint32_t hash = 10000;
                for (unsigned w=2; w < wordCount; ++w)
                    hash += w * hashType(typePos(spv[typeStart+w]));
                return hash;
            }

        case spv::OpTypeOpaque:         return 6000 + spv[typeStart+2];
        case spv::OpTypePointer:        return 100000  + hashType(typePos(spv[typeStart+3]));
        case spv::OpTypeFunction:
            {
                std::uint32_t hash = 200000;
                for (unsigned w=2; w < wordCount; ++w)
                    hash += w * hashType(typePos(spv[typeStart+w]));
                return hash;
            }

        case spv::OpTypeEvent:           return 300000;
        case spv::OpTypeDeviceEvent:     return 300001;
        case spv::OpTypeReserveId:       return 300002;
        case spv::OpTypeQueue:           return 300003;
        case spv::OpTypePipe:            return 300004;

        case spv::OpConstantNull:        return 300005;
        case spv::OpConstantSampler:     return 300006;

        case spv::OpConstantTrue:        return 300007;
        case spv::OpConstantFalse:       return 300008;
        case spv::OpConstantComposite:
            {
                std::uint32_t hash = 300011 + hashType(typePos(spv[typeStart+1]));
                for (unsigned w=3; w < wordCount; ++w)
                    hash += w * hashType(typePos(spv[typeStart+w]));
                return hash;
            }
        case spv::OpConstant:
            {
                std::uint32_t hash = 400011 + hashType(typePos(spv[typeStart+1]));
                for (unsigned w=3; w < wordCount; ++w)
                    hash += w * spv[typeStart+w];
                return hash;
            }

        default:
            error("unknown type opcode");
            return 0;
        }
    }

    void spirvbin_t::mapTypeConst()
    {
        globaltypes_t globalTypeMap;

        msg(3, 2, std::string("Remapping Consts & Types: "));

        static const std::uint32_t softTypeIdLimit = 3011;         static const std::uint32_t firstMappedID   = 8;    
        for (auto& typeStart : typeConstPos) {
            const spv::Id       resId     = asTypeConstId(typeStart);
            const std::uint32_t hashval   = hashType(typeStart);

            if (isOldIdUnmapped(resId))
                localId(resId, nextUnusedId(hashval % softTypeIdLimit + firstMappedID));
        }
    }


        void spirvbin_t::strip()
    {
        if (stripRange.empty())             return;

                std::sort(stripRange.begin(), stripRange.end());

                        auto strip_it = stripRange.begin();

        int strippedPos = 0;
        for (unsigned word = 0; word < unsigned(spv.size()); ++word) {
            if (strip_it != stripRange.end() && word >= strip_it->second)
                ++strip_it;

            if (strip_it == stripRange.end() || word < strip_it->first || word >= strip_it->second)
                spv[strippedPos++] = spv[word];
        }

        spv.resize(strippedPos);
        stripRange.clear();

        buildLocalMaps();
    }

        void spirvbin_t::remap(std::uint32_t opts)
    {
        options = opts;

                spv::Parameterize();

        validate();          buildLocalMaps();

        msg(3, 4, std::string("ID bound: ") + std::to_string(bound()));

        strip();        
        if (options & OPT_LOADSTORE) optLoadStore();
        if (options & OPT_FWD_LS)    forwardLoadStores();
        if (options & DCE_FUNCS)     dceFuncs();
        if (options & DCE_VARS)      dceVars();
        if (options & DCE_TYPES)     dceTypes();
        if (options & MAP_TYPES)     mapTypeConst();
        if (options & MAP_NAMES)     mapNames();
        if (options & MAP_FUNCS)     mapFnBodies();

        mapRemainder();         applyMap();             strip();            }

        void spirvbin_t::remap(std::vector<std::uint32_t>& in_spv, std::uint32_t opts)
    {
        spv.swap(in_spv);
        remap(opts);
        spv.swap(in_spv);
    }

} 
#endif 
