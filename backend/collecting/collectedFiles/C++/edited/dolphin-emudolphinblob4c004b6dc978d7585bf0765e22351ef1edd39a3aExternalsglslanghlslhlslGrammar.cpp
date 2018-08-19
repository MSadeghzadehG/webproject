

#include "hlslTokens.h"
#include "hlslGrammar.h"

namespace glslang {

bool HlslGrammar::parse()
{
    advanceToken();
    return acceptCompilationUnit();
}

void HlslGrammar::expected(const char* syntax)
{
    parseContext.error(token.loc, "Expected", syntax, "");
}

void HlslGrammar::unimplemented(const char* error)
{
    parseContext.error(token.loc, "Unimplemented", error, "");
}

bool HlslGrammar::acceptIdentifier(HlslToken& idToken)
{
    if (peekTokenClass(EHTokIdentifier)) {
        idToken = token;
        advanceToken();
        return true;
    }

    return false;
}

bool HlslGrammar::acceptCompilationUnit()
{
    TIntermNode* unitNode = nullptr;

    while (! peekTokenClass(EHTokNone)) {
                TIntermNode* declarationNode;
        if (! acceptDeclaration(declarationNode))
            return false;

                unitNode = intermediate.growAggregate(unitNode, declarationNode);
    }

        intermediate.setTreeRoot(unitNode);

    return true;
}

bool HlslGrammar::acceptSamplerState()
{
        
    if (! acceptTokenClass(EHTokLeftBrace))
        return true;

    parseContext.warn(token.loc, "unimplemented", "immediate sampler state", "");
    
    do {
                HlslToken state;
        if (! acceptIdentifier(state))
            break;  
                TString stateName = *state.string;
        std::transform(stateName.begin(), stateName.end(), stateName.begin(), ::tolower);

        if (! acceptTokenClass(EHTokAssign)) {
            expected("assign");
            return false;
        }

        if (stateName == "minlod" || stateName == "maxlod") {
            if (! peekTokenClass(EHTokIntConstant)) {
                expected("integer");
                return false;
            }

            TIntermTyped* lod = nullptr;
            if (! acceptLiteral(lod))                  return false;
        } else if (stateName == "maxanisotropy") {
            if (! peekTokenClass(EHTokIntConstant)) {
                expected("integer");
                return false;
            }

            TIntermTyped* maxAnisotropy = nullptr;
            if (! acceptLiteral(maxAnisotropy))                  return false;
        } else if (stateName == "filter") {
            HlslToken filterMode;
            if (! acceptIdentifier(filterMode)) {
                expected("filter mode");
                return false;
            }
        } else if (stateName == "addressu" || stateName == "addressv" || stateName == "addressw") {
            HlslToken addrMode;
            if (! acceptIdentifier(addrMode)) {
                expected("texture address mode");
                return false;
            }
        } else if (stateName == "miplodbias") {
            TIntermTyped* lodBias = nullptr;
            if (! acceptLiteral(lodBias)) {
                expected("lod bias");
                return false;
            }
        } else if (stateName == "bordercolor") {
            return false;
        } else {
            expected("texture state");
            return false;
        }

                if (! acceptTokenClass(EHTokSemicolon)) {
            expected("semicolon");
            return false;
        }
    } while (true);

    if (! acceptTokenClass(EHTokRightBrace))
        return false;

    return true;
}

bool HlslGrammar::acceptSamplerDeclarationDX9(TType& )
{
    if (! acceptTokenClass(EHTokSampler))
        return false;
 
        unimplemented("Direct3D 9 sampler declaration");

        HlslToken name;
    if (! acceptIdentifier(name)) {
        expected("sampler name");
        return false;
    }

    if (! acceptTokenClass(EHTokAssign)) {
        expected("=");
        return false;
    }

    return false;
}


bool HlslGrammar::acceptDeclaration(TIntermNode*& node)
{
    node = nullptr;
    bool list = false;

        bool typedefDecl = acceptTokenClass(EHTokTypedef);

    TType type;

        if (acceptSamplerDeclarationDX9(type))
        return true;

        if (! acceptFullySpecifiedType(type))
        return false;

    if (type.getQualifier().storage == EvqTemporary && parseContext.symbolTable.atGlobalLevel()) {
        if (type.getBasicType() == EbtSampler) {
                                                type.getQualifier().storage = EvqUniform; 
        } else {
            type.getQualifier().storage = EvqGlobal;
        }
    }

        HlslToken idToken;
    while (acceptIdentifier(idToken)) {
                TFunction* function = new TFunction(idToken.string, type);
        if (acceptFunctionParameters(*function)) {
                        acceptPostDecls(type);

                        if (peekTokenClass(EHTokLeftBrace)) {
                if (list)
                    parseContext.error(idToken.loc, "function body can't be in a declarator list", "{", "");
                if (typedefDecl)
                    parseContext.error(idToken.loc, "function body can't be in a typedef", "{", "");
                return acceptFunctionDefinition(*function, node);
            } else {
                if (typedefDecl)
                    parseContext.error(idToken.loc, "function typedefs not implemented", "{", "");
                parseContext.handleFunctionDeclarator(idToken.loc, *function, true);
            }
        } else {
            
                        TArraySizes* arraySizes = nullptr;
            acceptArraySpecifier(arraySizes);

                        if (type.getBasicType() == EbtSampler) {
                if (! acceptSamplerState())
                    return false;
            }

                        acceptPostDecls(type);

                        TIntermTyped* expressionNode = nullptr;
            if (acceptTokenClass(EHTokAssign)) {
                if (typedefDecl)
                    parseContext.error(idToken.loc, "can't have an initializer", "typedef", "");
                if (! acceptAssignmentExpression(expressionNode)) {
                    expected("initializer");
                    return false;
                }
            }

            if (typedefDecl)
                parseContext.declareTypedef(idToken.loc, *idToken.string, type, arraySizes);
            else {
                                                                node = intermediate.growAggregate(node,
                                                  parseContext.declareVariable(idToken.loc, *idToken.string, type,
                                                                               arraySizes, expressionNode),
                                                  idToken.loc);
            }
        }

        if (acceptTokenClass(EHTokComma)) {
            list = true;
            continue;
        }
    };

        if (node != nullptr)
        node->getAsAggregate()->setOperator(EOpSequence);

        if (! acceptTokenClass(EHTokSemicolon)) {
        expected(";");
        return false;
    }
    
    return true;
}

bool HlslGrammar::acceptControlDeclaration(TIntermNode*& node)
{
    node = nullptr;

        TType type;
    if (! acceptFullySpecifiedType(type))
        return false;

        HlslToken idToken;
    if (! acceptIdentifier(idToken)) {
        expected("identifier");
        return false;
    }

        TIntermTyped* expressionNode = nullptr;
    if (! acceptTokenClass(EHTokAssign)) {
        expected("=");
        return false;
    }

        if (! acceptExpression(expressionNode)) {
        expected("initializer");
        return false;
    }

    node = parseContext.declareVariable(idToken.loc, *idToken.string, type, 0, expressionNode);

    return true;
}

bool HlslGrammar::acceptFullySpecifiedType(TType& type)
{
        TQualifier qualifier;
    qualifier.clear();
    acceptQualifier(qualifier);

        if (! acceptType(type))
        return false;
    type.getQualifier() = qualifier;

    return true;
}

void HlslGrammar::acceptQualifier(TQualifier& qualifier)
{
    do {
        switch (peek()) {
        case EHTokStatic:
                        break;
        case EHTokExtern:
                        break;
        case EHTokShared:
                        break;
        case EHTokGroupShared:
            qualifier.storage = EvqShared;
            break;
        case EHTokUniform:
            qualifier.storage = EvqUniform;
            break;
        case EHTokConst:
            qualifier.storage = EvqConst;
            break;
        case EHTokVolatile:
            qualifier.volatil = true;
            break;
        case EHTokLinear:
            qualifier.storage = EvqVaryingIn;
            qualifier.smooth = true;
            break;
        case EHTokCentroid:
            qualifier.centroid = true;
            break;
        case EHTokNointerpolation:
            qualifier.flat = true;
            break;
        case EHTokNoperspective:
            qualifier.nopersp = true;
            break;
        case EHTokSample:
            qualifier.sample = true;
            break;
        case EHTokRowMajor:
            qualifier.layoutMatrix = ElmRowMajor;
            break;
        case EHTokColumnMajor:
            qualifier.layoutMatrix = ElmColumnMajor;
            break;
        case EHTokPrecise:
            qualifier.noContraction = true;
            break;
        case EHTokIn:
            qualifier.storage = EvqIn;
            break;
        case EHTokOut:
            qualifier.storage = EvqOut;
            break;
        case EHTokInOut:
            qualifier.storage = EvqInOut;
            break;
        default:
            return;
        }
        advanceToken();
    } while (true);
}

bool HlslGrammar::acceptTemplateType(TBasicType& basicType)
{
    switch (peek()) {
    case EHTokFloat:
        basicType = EbtFloat;
        break;
    case EHTokDouble:
        basicType = EbtDouble;
        break;
    case EHTokInt:
    case EHTokDword:
        basicType = EbtInt;
        break;
    case EHTokUint:
        basicType = EbtUint;
        break;
    case EHTokBool:
        basicType = EbtBool;
        break;
    default:
        return false;
    }

    advanceToken();

    return true;
}

bool HlslGrammar::acceptVectorTemplateType(TType& type)
{
    if (! acceptTokenClass(EHTokVector))
        return false;

    if (! acceptTokenClass(EHTokLeftAngle)) {
                new(&type) TType(EbtFloat, EvqTemporary, 4);
        return true;
    }

    TBasicType basicType;
    if (! acceptTemplateType(basicType)) {
        expected("scalar type");
        return false;
    }

        if (! acceptTokenClass(EHTokComma)) {
        expected(",");
        return false;
    }

        if (! peekTokenClass(EHTokIntConstant)) {
        expected("literal integer");
        return false;
    }

    TIntermTyped* vecSize;
    if (! acceptLiteral(vecSize))
        return false;

    const int vecSizeI = vecSize->getAsConstantUnion()->getConstArray()[0].getIConst();

    new(&type) TType(basicType, EvqTemporary, vecSizeI);

    if (vecSizeI == 1)
        type.makeVector();

    if (!acceptTokenClass(EHTokRightAngle)) {
        expected("right angle bracket");
        return false;
    }

    return true;
}

bool HlslGrammar::acceptMatrixTemplateType(TType& type)
{
    if (! acceptTokenClass(EHTokMatrix))
        return false;

    if (! acceptTokenClass(EHTokLeftAngle)) {
                new(&type) TType(EbtFloat, EvqTemporary, 0, 4, 4);
        return true;
    }

    TBasicType basicType;
    if (! acceptTemplateType(basicType)) {
        expected("scalar type");
        return false;
    }

        if (! acceptTokenClass(EHTokComma)) {
        expected(",");
        return false;
    }

        if (! peekTokenClass(EHTokIntConstant)) {
        expected("literal integer");
        return false;
    }

    TIntermTyped* rows;
    if (! acceptLiteral(rows))
        return false;

        if (! acceptTokenClass(EHTokComma)) {
        expected(",");
        return false;
    }
    
        if (! peekTokenClass(EHTokIntConstant)) {
        expected("literal integer");
        return false;
    }

    TIntermTyped* cols;
    if (! acceptLiteral(cols))
        return false;

    new(&type) TType(basicType, EvqTemporary, 0,
                     cols->getAsConstantUnion()->getConstArray()[0].getIConst(),
                     rows->getAsConstantUnion()->getConstArray()[0].getIConst());

    if (!acceptTokenClass(EHTokRightAngle)) {
        expected("right angle bracket");
        return false;
    }

    return true;
}


bool HlslGrammar::acceptSamplerType(TType& type)
{
        const EHlslTokenClass samplerType = peek();

    TSamplerDim dim = EsdNone;

    switch (samplerType) {
    case EHTokSampler:      break;
    case EHTokSampler1d:    dim = Esd1D; break;
    case EHTokSampler2d:    dim = Esd2D; break;
    case EHTokSampler3d:    dim = Esd3D; break;
    case EHTokSamplerCube:  dim = EsdCube; break;
    case EHTokSamplerState: break;
    case EHTokSamplerComparisonState: break;
    default:
        return false;      }

    advanceToken();  
    TArraySizes* arraySizes = nullptr;     bool shadow = false;               
    TSampler sampler;
    sampler.setPureSampler(shadow);

    type.shallowCopy(TType(sampler, EvqUniform, arraySizes));

    return true;
}

bool HlslGrammar::acceptTextureType(TType& type)
{
    const EHlslTokenClass textureType = peek();

    TSamplerDim dim = EsdNone;
    bool array = false;
    bool ms    = false;

    switch (textureType) {
    case EHTokBuffer:            dim = EsdBuffer;                      break;
    case EHTokTexture1d:         dim = Esd1D;                          break;
    case EHTokTexture1darray:    dim = Esd1D; array = true;            break;
    case EHTokTexture2d:         dim = Esd2D;                          break;
    case EHTokTexture2darray:    dim = Esd2D; array = true;            break;
    case EHTokTexture3d:         dim = Esd3D;                          break; 
    case EHTokTextureCube:       dim = EsdCube;                        break;
    case EHTokTextureCubearray:  dim = EsdCube; array = true;          break;
    case EHTokTexture2DMS:       dim = Esd2D; ms = true;               break;
    case EHTokTexture2DMSarray:  dim = Esd2D; array = true; ms = true; break;
    default:
        return false;      }

    advanceToken();  
    TType txType(EbtFloat, EvqUniform, 4);     
    TIntermTyped* msCount = nullptr;

        if (acceptTokenClass(EHTokLeftAngle)) {
        if (! acceptType(txType)) {
            expected("scalar or vector type");
            return false;
        }

        const TBasicType basicRetType = txType.getBasicType() ;

        if (basicRetType != EbtFloat && basicRetType != EbtUint && basicRetType != EbtInt) {
            unimplemented("basic type in texture");
            return false;
        }

        if (!txType.isScalar() && !txType.isVector()) {
            expected("scalar or vector type");
            return false;
        }

        if (txType.getVectorSize() != 1 && txType.getVectorSize() != 4) {
                        expected("vector size not yet supported in texture type");
            return false;
        }

        if (ms && acceptTokenClass(EHTokComma)) {
                        if (! peekTokenClass(EHTokIntConstant)) {
                expected("multisample count");
                return false;
            }

            if (! acceptLiteral(msCount))                  return false;
        }

        if (! acceptTokenClass(EHTokRightAngle)) {
            expected("right angle bracket");
            return false;
        }
    } else if (ms) {
        expected("texture type for multisample");
        return false;
    }

    TArraySizes* arraySizes = nullptr;
    const bool shadow = txType.isScalar() || (txType.isVector() && txType.getVectorSize() == 1);

    TSampler sampler;
    sampler.setTexture(txType.getBasicType(), dim, array, shadow, ms);
    
    type.shallowCopy(TType(sampler, EvqUniform, arraySizes));

    return true;
}


bool HlslGrammar::acceptType(TType& type)
{
    switch (peek()) {
    case EHTokVector:
        return acceptVectorTemplateType(type);
        break;

    case EHTokMatrix:
        return acceptMatrixTemplateType(type);
        break;

    case EHTokSampler:                    case EHTokSampler1d:                  case EHTokSampler2d:                  case EHTokSampler3d:                  case EHTokSamplerCube:                case EHTokSamplerState:               case EHTokSamplerComparisonState:         return acceptSamplerType(type);
        break;

    case EHTokBuffer:                     case EHTokTexture1d:                  case EHTokTexture1darray:             case EHTokTexture2d:                  case EHTokTexture2darray:             case EHTokTexture3d:                  case EHTokTextureCube:                case EHTokTextureCubearray:           case EHTokTexture2DMS:                case EHTokTexture2DMSarray:               return acceptTextureType(type);
        break;

    case EHTokStruct:
        return acceptStruct(type);
        break;

    case EHTokIdentifier:
                                token.symbol = parseContext.symbolTable.find(*token.string);
        if (token.symbol && token.symbol->getAsVariable() && token.symbol->getAsVariable()->isUserType()) {
            type.shallowCopy(token.symbol->getType());
            advanceToken();
            return true;
        } else
            return false;

    case EHTokVoid:
        new(&type) TType(EbtVoid);
        break;

    case EHTokFloat:
        new(&type) TType(EbtFloat);
        break;
    case EHTokFloat1:
        new(&type) TType(EbtFloat);
        type.makeVector();
        break;
    case EHTokFloat2:
        new(&type) TType(EbtFloat, EvqTemporary, 2);
        break;
    case EHTokFloat3:
        new(&type) TType(EbtFloat, EvqTemporary, 3);
        break;
    case EHTokFloat4:
        new(&type) TType(EbtFloat, EvqTemporary, 4);
        break;

    case EHTokDouble:
        new(&type) TType(EbtDouble);
        break;
    case EHTokDouble1:
        new(&type) TType(EbtDouble);
        type.makeVector();
        break;
    case EHTokDouble2:
        new(&type) TType(EbtDouble, EvqTemporary, 2);
        break;
    case EHTokDouble3:
        new(&type) TType(EbtDouble, EvqTemporary, 3);
        break;
    case EHTokDouble4:
        new(&type) TType(EbtDouble, EvqTemporary, 4);
        break;

    case EHTokInt:
    case EHTokDword:
        new(&type) TType(EbtInt);
        break;
    case EHTokInt1:
        new(&type) TType(EbtInt);
        type.makeVector();
        break;
    case EHTokInt2:
        new(&type) TType(EbtInt, EvqTemporary, 2);
        break;
    case EHTokInt3:
        new(&type) TType(EbtInt, EvqTemporary, 3);
        break;
    case EHTokInt4:
        new(&type) TType(EbtInt, EvqTemporary, 4);
        break;

    case EHTokUint:
        new(&type) TType(EbtUint);
        break;
    case EHTokUint1:
        new(&type) TType(EbtUint);
        type.makeVector();
        break;
    case EHTokUint2:
        new(&type) TType(EbtUint, EvqTemporary, 2);
        break;
    case EHTokUint3:
        new(&type) TType(EbtUint, EvqTemporary, 3);
        break;
    case EHTokUint4:
        new(&type) TType(EbtUint, EvqTemporary, 4);
        break;


    case EHTokBool:
        new(&type) TType(EbtBool);
        break;
    case EHTokBool1:
        new(&type) TType(EbtBool);
        type.makeVector();
        break;
    case EHTokBool2:
        new(&type) TType(EbtBool, EvqTemporary, 2);
        break;
    case EHTokBool3:
        new(&type) TType(EbtBool, EvqTemporary, 3);
        break;
    case EHTokBool4:
        new(&type) TType(EbtBool, EvqTemporary, 4);
        break;

    case EHTokInt1x1:
        new(&type) TType(EbtInt, EvqTemporary, 0, 1, 1);
        break;
    case EHTokInt1x2:
        new(&type) TType(EbtInt, EvqTemporary, 0, 2, 1);
        break;
    case EHTokInt1x3:
        new(&type) TType(EbtInt, EvqTemporary, 0, 3, 1);
        break;
    case EHTokInt1x4:
        new(&type) TType(EbtInt, EvqTemporary, 0, 4, 1);
        break;
    case EHTokInt2x1:
        new(&type) TType(EbtInt, EvqTemporary, 0, 1, 2);
        break;
    case EHTokInt2x2:
        new(&type) TType(EbtInt, EvqTemporary, 0, 2, 2);
        break;
    case EHTokInt2x3:
        new(&type) TType(EbtInt, EvqTemporary, 0, 3, 2);
        break;
    case EHTokInt2x4:
        new(&type) TType(EbtInt, EvqTemporary, 0, 4, 2);
        break;
    case EHTokInt3x1:
        new(&type) TType(EbtInt, EvqTemporary, 0, 1, 3);
        break;
    case EHTokInt3x2:
        new(&type) TType(EbtInt, EvqTemporary, 0, 2, 3);
        break;
    case EHTokInt3x3:
        new(&type) TType(EbtInt, EvqTemporary, 0, 3, 3);
        break;
    case EHTokInt3x4:
        new(&type) TType(EbtInt, EvqTemporary, 0, 4, 3);
        break;
    case EHTokInt4x1:
        new(&type) TType(EbtInt, EvqTemporary, 0, 1, 4);
        break;
    case EHTokInt4x2:
        new(&type) TType(EbtInt, EvqTemporary, 0, 2, 4);
        break;
    case EHTokInt4x3:
        new(&type) TType(EbtInt, EvqTemporary, 0, 3, 4);
        break;
    case EHTokInt4x4:
        new(&type) TType(EbtInt, EvqTemporary, 0, 4, 4);
        break;

    case EHTokUint1x1:
        new(&type) TType(EbtUint, EvqTemporary, 0, 1, 1);
        break;
    case EHTokUint1x2:
        new(&type) TType(EbtUint, EvqTemporary, 0, 2, 1);
        break;
    case EHTokUint1x3:
        new(&type) TType(EbtUint, EvqTemporary, 0, 3, 1);
        break;
    case EHTokUint1x4:
        new(&type) TType(EbtUint, EvqTemporary, 0, 4, 1);
        break;
    case EHTokUint2x1:
        new(&type) TType(EbtUint, EvqTemporary, 0, 1, 2);
        break;
    case EHTokUint2x2:
        new(&type) TType(EbtUint, EvqTemporary, 0, 2, 2);
        break;
    case EHTokUint2x3:
        new(&type) TType(EbtUint, EvqTemporary, 0, 3, 2);
        break;
    case EHTokUint2x4:
        new(&type) TType(EbtUint, EvqTemporary, 0, 4, 2);
        break;
    case EHTokUint3x1:
        new(&type) TType(EbtUint, EvqTemporary, 0, 1, 3);
        break;
    case EHTokUint3x2:
        new(&type) TType(EbtUint, EvqTemporary, 0, 2, 3);
        break;
    case EHTokUint3x3:
        new(&type) TType(EbtUint, EvqTemporary, 0, 3, 3);
        break;
    case EHTokUint3x4:
        new(&type) TType(EbtUint, EvqTemporary, 0, 4, 3);
        break;
    case EHTokUint4x1:
        new(&type) TType(EbtUint, EvqTemporary, 0, 1, 4);
        break;
    case EHTokUint4x2:
        new(&type) TType(EbtUint, EvqTemporary, 0, 2, 4);
        break;
    case EHTokUint4x3:
        new(&type) TType(EbtUint, EvqTemporary, 0, 3, 4);
        break;
    case EHTokUint4x4:
        new(&type) TType(EbtUint, EvqTemporary, 0, 4, 4);
        break;

    case EHTokBool1x1:
        new(&type) TType(EbtBool, EvqTemporary, 0, 1, 1);
        break;
    case EHTokBool1x2:
        new(&type) TType(EbtBool, EvqTemporary, 0, 2, 1);
        break;
    case EHTokBool1x3:
        new(&type) TType(EbtBool, EvqTemporary, 0, 3, 1);
        break;
    case EHTokBool1x4:
        new(&type) TType(EbtBool, EvqTemporary, 0, 4, 1);
        break;
    case EHTokBool2x1:
        new(&type) TType(EbtBool, EvqTemporary, 0, 1, 2);
        break;
    case EHTokBool2x2:
        new(&type) TType(EbtBool, EvqTemporary, 0, 2, 2);
        break;
    case EHTokBool2x3:
        new(&type) TType(EbtBool, EvqTemporary, 0, 3, 2);
        break;
    case EHTokBool2x4:
        new(&type) TType(EbtBool, EvqTemporary, 0, 4, 2);
        break;
    case EHTokBool3x1:
        new(&type) TType(EbtBool, EvqTemporary, 0, 1, 3);
        break;
    case EHTokBool3x2:
        new(&type) TType(EbtBool, EvqTemporary, 0, 2, 3);
        break;
    case EHTokBool3x3:
        new(&type) TType(EbtBool, EvqTemporary, 0, 3, 3);
        break;
    case EHTokBool3x4:
        new(&type) TType(EbtBool, EvqTemporary, 0, 4, 3);
        break;
    case EHTokBool4x1:
        new(&type) TType(EbtBool, EvqTemporary, 0, 1, 4);
        break;
    case EHTokBool4x2:
        new(&type) TType(EbtBool, EvqTemporary, 0, 2, 4);
        break;
    case EHTokBool4x3:
        new(&type) TType(EbtBool, EvqTemporary, 0, 3, 4);
        break;
    case EHTokBool4x4:
        new(&type) TType(EbtBool, EvqTemporary, 0, 4, 4);
        break;

    case EHTokFloat1x1:
        new(&type) TType(EbtFloat, EvqTemporary, 0, 1, 1);
        break;
    case EHTokFloat1x2:
        new(&type) TType(EbtFloat, EvqTemporary, 0, 2, 1);
        break;
    case EHTokFloat1x3:
        new(&type) TType(EbtFloat, EvqTemporary, 0, 3, 1);
        break;
    case EHTokFloat1x4:
        new(&type) TType(EbtFloat, EvqTemporary, 0, 4, 1);
        break;
    case EHTokFloat2x1:
        new(&type) TType(EbtFloat, EvqTemporary, 0, 1, 2);
        break;
    case EHTokFloat2x2:
        new(&type) TType(EbtFloat, EvqTemporary, 0, 2, 2);
        break;
    case EHTokFloat2x3:
        new(&type) TType(EbtFloat, EvqTemporary, 0, 3, 2);
        break;
    case EHTokFloat2x4:
        new(&type) TType(EbtFloat, EvqTemporary, 0, 4, 2);
        break;
    case EHTokFloat3x1:
        new(&type) TType(EbtFloat, EvqTemporary, 0, 1, 3);
        break;
    case EHTokFloat3x2:
        new(&type) TType(EbtFloat, EvqTemporary, 0, 2, 3);
        break;
    case EHTokFloat3x3:
        new(&type) TType(EbtFloat, EvqTemporary, 0, 3, 3);
        break;
    case EHTokFloat3x4:
        new(&type) TType(EbtFloat, EvqTemporary, 0, 4, 3);
        break;
    case EHTokFloat4x1:
        new(&type) TType(EbtFloat, EvqTemporary, 0, 1, 4);
        break;
    case EHTokFloat4x2:
        new(&type) TType(EbtFloat, EvqTemporary, 0, 2, 4);
        break;
    case EHTokFloat4x3:
        new(&type) TType(EbtFloat, EvqTemporary, 0, 3, 4);
        break;
    case EHTokFloat4x4:
        new(&type) TType(EbtFloat, EvqTemporary, 0, 4, 4);
        break;

    case EHTokDouble1x1:
        new(&type) TType(EbtDouble, EvqTemporary, 0, 1, 1);
        break;
    case EHTokDouble1x2:
        new(&type) TType(EbtDouble, EvqTemporary, 0, 2, 1);
        break;
    case EHTokDouble1x3:
        new(&type) TType(EbtDouble, EvqTemporary, 0, 3, 1);
        break;
    case EHTokDouble1x4:
        new(&type) TType(EbtDouble, EvqTemporary, 0, 4, 1);
        break;
    case EHTokDouble2x1:
        new(&type) TType(EbtDouble, EvqTemporary, 0, 1, 2);
        break;
    case EHTokDouble2x2:
        new(&type) TType(EbtDouble, EvqTemporary, 0, 2, 2);
        break;
    case EHTokDouble2x3:
        new(&type) TType(EbtDouble, EvqTemporary, 0, 3, 2);
        break;
    case EHTokDouble2x4:
        new(&type) TType(EbtDouble, EvqTemporary, 0, 4, 2);
        break;
    case EHTokDouble3x1:
        new(&type) TType(EbtDouble, EvqTemporary, 0, 1, 3);
        break;
    case EHTokDouble3x2:
        new(&type) TType(EbtDouble, EvqTemporary, 0, 2, 3);
        break;
    case EHTokDouble3x3:
        new(&type) TType(EbtDouble, EvqTemporary, 0, 3, 3);
        break;
    case EHTokDouble3x4:
        new(&type) TType(EbtDouble, EvqTemporary, 0, 4, 3);
        break;
    case EHTokDouble4x1:
        new(&type) TType(EbtDouble, EvqTemporary, 0, 1, 4);
        break;
    case EHTokDouble4x2:
        new(&type) TType(EbtDouble, EvqTemporary, 0, 2, 4);
        break;
    case EHTokDouble4x3:
        new(&type) TType(EbtDouble, EvqTemporary, 0, 3, 4);
        break;
    case EHTokDouble4x4:
        new(&type) TType(EbtDouble, EvqTemporary, 0, 4, 4);
        break;

    default:
        return false;
    }

    advanceToken();

    return true;
}

bool HlslGrammar::acceptStruct(TType& type)
{
        if (! acceptTokenClass(EHTokStruct))
        return false;

        TString structName = "";
    if (peekTokenClass(EHTokIdentifier)) {
        structName = *token.string;
        advanceToken();
    }

        if (! acceptTokenClass(EHTokLeftBrace)) {
        expected("{");
        return false;
    }

        TTypeList* typeList;
    if (! acceptStructDeclarationList(typeList)) {
        expected("struct member declarations");
        return false;
    }

        if (! acceptTokenClass(EHTokRightBrace)) {
        expected("}");
        return false;
    }

        new(&type) TType(typeList, structName);

            if (structName.size() > 0) {
        TVariable* userTypeDef = new TVariable(&structName, type, true);
        if (! parseContext.symbolTable.insert(*userTypeDef))
            parseContext.error(token.loc, "redefinition", structName.c_str(), "struct");
    }

    return true;
}

bool HlslGrammar::acceptStructDeclarationList(TTypeList*& typeList)
{
    typeList = new TTypeList();

    do {
                if (peekTokenClass(EHTokRightBrace))
            return true;

        
                TType memberType;
        if (! acceptFullySpecifiedType(memberType)) {
            expected("member type");
            return false;
        }

                do {
                        if (! peekTokenClass(EHTokIdentifier)) {
                expected("member name");
                return false;
            }

                        TTypeLoc member = { new TType(EbtVoid), token.loc };
            member.type->shallowCopy(memberType);
            member.type->setFieldName(*token.string);
            typeList->push_back(member);

                        advanceToken();

                        TArraySizes* arraySizes = nullptr;
            acceptArraySpecifier(arraySizes);
            if (arraySizes)
                typeList->back().type->newArraySizes(*arraySizes);

            acceptPostDecls(*member.type);

                        if (peekTokenClass(EHTokSemicolon))
                break;

                        if (! acceptTokenClass(EHTokComma)) {
                expected(",");
                return false;
            }

        } while (true);

                if (! acceptTokenClass(EHTokSemicolon)) {
            expected(";");
            return false;
        }

    } while (true);
}

bool HlslGrammar::acceptFunctionParameters(TFunction& function)
{
        if (! acceptTokenClass(EHTokLeftParen))
        return false;

        if (! acceptTokenClass(EHTokVoid)) {
        do {
                        if (! acceptParameterDeclaration(function))
                break;

                        if (! acceptTokenClass(EHTokComma))
                break;
        } while (true);
    }

        if (! acceptTokenClass(EHTokRightParen)) {
        expected(")");
        return false;
    }

    return true;
}

bool HlslGrammar::acceptParameterDeclaration(TFunction& function)
{
        TType* type = new TType;
    if (! acceptFullySpecifiedType(*type))
        return false;

        HlslToken idToken;
    acceptIdentifier(idToken);

        TArraySizes* arraySizes = nullptr;
    acceptArraySpecifier(arraySizes);
    if (arraySizes)
        type->newArraySizes(*arraySizes);

        acceptPostDecls(*type);

    parseContext.paramFix(*type);

    TParameter param = { idToken.string, type };
    function.addParameter(param);

    return true;
}

bool HlslGrammar::acceptFunctionDefinition(TFunction& function, TIntermNode*& node)
{
    TFunction* functionDeclarator = parseContext.handleFunctionDeclarator(token.loc, function, false );

        node = parseContext.handleFunctionDefinition(token.loc, *functionDeclarator);

        TIntermNode* functionBody = nullptr;
    if (acceptCompoundStatement(functionBody)) {
        node = intermediate.growAggregate(node, functionBody);
        intermediate.setAggregateOperator(node, EOpFunction, functionDeclarator->getType(), token.loc);
        node->getAsAggregate()->setName(functionDeclarator->getMangledName().c_str());
        parseContext.popScope();

        return true;
    }

    return false;
}

bool HlslGrammar::acceptParenExpression(TIntermTyped*& expression)
{
        if (! acceptTokenClass(EHTokLeftParen))
        expected("(");

    bool decl = false;
    TIntermNode* declNode = nullptr;
    decl = acceptControlDeclaration(declNode);
    if (decl) {
        if (declNode == nullptr || declNode->getAsTyped() == nullptr) {
            expected("initialized declaration");
            return false;
        } else
            expression = declNode->getAsTyped();
    } else {
                if (! acceptExpression(expression)) {
            expected("expression");
            return false;
        }
    }

        if (! acceptTokenClass(EHTokRightParen))
        expected(")");

    return true;
}

bool HlslGrammar::acceptExpression(TIntermTyped*& node)
{
    node = nullptr;

        if (! acceptAssignmentExpression(node))
        return false;

    if (! peekTokenClass(EHTokComma))
        return true;

    do {
                TSourceLoc loc = token.loc;
        advanceToken();

                TIntermTyped* rightNode = nullptr;
        if (! acceptAssignmentExpression(rightNode)) {
            expected("assignment expression");
            return false;
        }

        node = intermediate.addComma(node, rightNode, loc);

        if (! peekTokenClass(EHTokComma))
            return true;
    } while (true);
}

bool HlslGrammar::acceptInitializer(TIntermTyped*& node)
{
        if (! acceptTokenClass(EHTokLeftBrace))
        return false;

        TSourceLoc loc = token.loc;
    node = nullptr;
    do {
                TIntermTyped* expr;
        if (! acceptAssignmentExpression(expr)) {
            expected("assignment expression in initializer list");
            return false;
        }
        node = intermediate.growAggregate(node, expr, loc);

                if (acceptTokenClass(EHTokComma))
            continue;

                if (acceptTokenClass(EHTokRightBrace))
            return true;

        expected(", or }");
        return false;
    } while (true);
}

bool HlslGrammar::acceptAssignmentExpression(TIntermTyped*& node)
{
        if (peekTokenClass(EHTokLeftBrace)) {
        if (acceptInitializer(node))
            return true;

        expected("initializer");
        return false;
    }

        if (! acceptBinaryExpression(node, PlLogicalOr))
        return false;

        TOperator assignOp = HlslOpMap::assignment(peek());
    if (assignOp == EOpNull)
        return true;

        TSourceLoc loc = token.loc;
    advanceToken();

                TIntermTyped* rightNode = nullptr;
    if (! acceptAssignmentExpression(rightNode)) {
        expected("assignment expression");
        return false;
    }

    node = intermediate.addAssign(assignOp, node, rightNode, loc);

    if (! peekTokenClass(EHTokComma))
        return true;

    return true;
}

bool HlslGrammar::acceptBinaryExpression(TIntermTyped*& node, PrecedenceLevel precedenceLevel)
{
    if (precedenceLevel > PlMul)
        return acceptUnaryExpression(node);

        if (! acceptBinaryExpression(node, (PrecedenceLevel)(precedenceLevel + 1)))
        return false;

    TOperator op = HlslOpMap::binary(peek());
    PrecedenceLevel tokenLevel = HlslOpMap::precedenceLevel(op);
    if (tokenLevel < precedenceLevel)
        return true;

    do {
                TSourceLoc loc = token.loc;
        advanceToken();

                TIntermTyped* rightNode = nullptr;
        if (! acceptBinaryExpression(rightNode, (PrecedenceLevel)(precedenceLevel + 1))) {
            expected("expression");
            return false;
        }

        node = intermediate.addBinaryMath(op, node, rightNode, loc);

        if (! peekTokenClass(EHTokComma))
            return true;
    } while (true);
}

bool HlslGrammar::acceptUnaryExpression(TIntermTyped*& node)
{
                if (acceptTokenClass(EHTokLeftParen)) {
        TType castType;
        if (acceptType(castType)) {
            if (! acceptTokenClass(EHTokRightParen)) {
                expected(")");
                return false;
            }

                        TSourceLoc loc = token.loc;
            if (! acceptUnaryExpression(node))
                return false;

                        TFunction* constructorFunction = parseContext.handleConstructorCall(loc, castType);
            if (constructorFunction == nullptr) {
                expected("type that can be constructed");
                return false;
            }
            TIntermTyped* arguments = nullptr;
            parseContext.handleFunctionArgument(constructorFunction, arguments, node);
            node = parseContext.handleFunctionCall(loc, constructorFunction, arguments);

            return true;
        } else {
                                                recedeToken();
            return acceptPostfixExpression(node);
        }
    }

        TOperator unaryOp = HlslOpMap::preUnary(peek());
    
        if (unaryOp == EOpNull)
        return acceptPostfixExpression(node);

        TSourceLoc loc = token.loc;
    advanceToken();
    if (! acceptUnaryExpression(node))
        return false;

        if (unaryOp == EOpAdd)
        return true;

    node = intermediate.addUnaryMath(unaryOp, node, loc);

    return node != nullptr;
}

bool HlslGrammar::acceptPostfixExpression(TIntermTyped*& node)
{
        
        HlslToken idToken;

            if (acceptTokenClass(EHTokLeftParen)) {
                if (! acceptExpression(node)) {
            expected("expression");
            return false;
        }
        if (! acceptTokenClass(EHTokRightParen)) {
            expected(")");
            return false;
        }
    } else if (acceptLiteral(node)) {
            } else if (acceptConstructor(node)) {
            } else if (acceptIdentifier(idToken)) {
                if (! peekTokenClass(EHTokLeftParen)) {
            node = parseContext.handleVariable(idToken.loc, idToken.symbol, token.string);
        } else if (acceptFunctionCall(idToken, node)) {
                    } else {
            expected("function call arguments");
            return false;
        }
    } else {
                return false;
    }

        do {
        TSourceLoc loc = token.loc;
        TOperator postOp = HlslOpMap::postUnary(peek());

                switch (postOp) {
        case EOpIndexDirectStruct:
        case EOpIndexIndirect:
        case EOpPostIncrement:
        case EOpPostDecrement:
            advanceToken();
            break;
        default:
            return true;
        }

                switch (postOp) {
        case EOpIndexDirectStruct:
        {
                                    HlslToken field;
            if (! acceptIdentifier(field)) {
                expected("swizzle or member");
                return false;
            }

            TIntermTyped* base = node;             node = parseContext.handleDotDereference(field.loc, node, *field.string);

                        if (node->getAsMethodNode() != nullptr && peekTokenClass(EHTokLeftParen)) {
                if (! acceptFunctionCall(field, node, base)) {
                    expected("function parameters");
                    return false;
                }
            }

            break;
        }
        case EOpIndexIndirect:
        {
                        TIntermTyped* indexNode = nullptr;
            if (! acceptExpression(indexNode) ||
                ! peekTokenClass(EHTokRightBracket)) {
                expected("expression followed by ']'");
                return false;
            }
            advanceToken();
            node = parseContext.handleBracketDereference(indexNode->getLoc(), node, indexNode);
            break;
        }
        case EOpPostIncrement:
                                case EOpPostDecrement:
                        node = intermediate.addUnaryMath(postOp, node, loc);
            break;
        default:
            assert(0);
            break;
        }
    } while (true);
}

bool HlslGrammar::acceptConstructor(TIntermTyped*& node)
{
        TType type;
    if (acceptType(type)) {
        TFunction* constructorFunction = parseContext.handleConstructorCall(token.loc, type);
        if (constructorFunction == nullptr)
            return false;

                TIntermTyped* arguments = nullptr;
        if (! acceptArguments(constructorFunction, arguments)) {
            expected("constructor arguments");
            return false;
        }

                node = parseContext.handleFunctionCall(arguments->getLoc(), constructorFunction, arguments);

        return true;
    }

    return false;
}

bool HlslGrammar::acceptFunctionCall(HlslToken idToken, TIntermTyped*& node, TIntermTyped* base)
{
        TFunction* function = new TFunction(idToken.string, TType(EbtVoid));
    TIntermTyped* arguments = nullptr;

        if (base != nullptr)
        parseContext.handleFunctionArgument(function, arguments, base);

    if (! acceptArguments(function, arguments))
        return false;

    node = parseContext.handleFunctionCall(idToken.loc, function, arguments);

    return true;
}

bool HlslGrammar::acceptArguments(TFunction* function, TIntermTyped*& arguments)
{
        if (! acceptTokenClass(EHTokLeftParen))
        return false;

    do {
                TIntermTyped* arg;
        if (! acceptAssignmentExpression(arg))
            break;

                parseContext.handleFunctionArgument(function, arguments, arg);

                if (! acceptTokenClass(EHTokComma))
            break;
    } while (true);

        if (! acceptTokenClass(EHTokRightParen)) {
        expected(")");
        return false;
    }

    return true;
}

bool HlslGrammar::acceptLiteral(TIntermTyped*& node)
{
    switch (token.tokenClass) {
    case EHTokIntConstant:
        node = intermediate.addConstantUnion(token.i, token.loc, true);
        break;
    case EHTokFloatConstant:
        node = intermediate.addConstantUnion(token.d, EbtFloat, token.loc, true);
        break;
    case EHTokDoubleConstant:
        node = intermediate.addConstantUnion(token.d, EbtDouble, token.loc, true);
        break;
    case EHTokBoolConstant:
        node = intermediate.addConstantUnion(token.b, token.loc, true);
        break;

    default:
        return false;
    }

    advanceToken();

    return true;
}

bool HlslGrammar::acceptCompoundStatement(TIntermNode*& retStatement)
{
    TIntermAggregate* compoundStatement = nullptr;

        if (! acceptTokenClass(EHTokLeftBrace))
        return false;

        TIntermNode* statement = nullptr;
    while (acceptStatement(statement)) {
        TIntermBranch* branch = statement ? statement->getAsBranchNode() : nullptr;
        if (branch != nullptr && (branch->getFlowOp() == EOpCase ||
                                  branch->getFlowOp() == EOpDefault)) {
                        parseContext.wrapupSwitchSubsequence(compoundStatement, statement);
            compoundStatement = nullptr;
        } else {
                        compoundStatement = intermediate.growAggregate(compoundStatement, statement);
        }
    }
    if (compoundStatement)
        compoundStatement->setOperator(EOpSequence);

    retStatement = compoundStatement;

        return acceptTokenClass(EHTokRightBrace);
}

bool HlslGrammar::acceptScopedStatement(TIntermNode*& statement)
{
    parseContext.pushScope();
    bool result = acceptStatement(statement);
    parseContext.popScope();

    return result;
}

bool HlslGrammar::acceptScopedCompoundStatement(TIntermNode*& statement)
{
    parseContext.pushScope();
    bool result = acceptCompoundStatement(statement);
    parseContext.popScope();

    return result;
}

bool HlslGrammar::acceptStatement(TIntermNode*& statement)
{
    statement = nullptr;

        acceptAttributes();

        switch (peek()) {
    case EHTokLeftBrace:
        return acceptScopedCompoundStatement(statement);

    case EHTokIf:
        return acceptSelectionStatement(statement);

    case EHTokSwitch:
        return acceptSwitchStatement(statement);

    case EHTokFor:
    case EHTokDo:
    case EHTokWhile:
        return acceptIterationStatement(statement);

    case EHTokContinue:
    case EHTokBreak:
    case EHTokDiscard:
    case EHTokReturn:
        return acceptJumpStatement(statement);

    case EHTokCase:
        return acceptCaseLabel(statement);
    case EHTokDefault:
        return acceptDefaultLabel(statement);

    case EHTokSemicolon:
        return acceptTokenClass(EHTokSemicolon);

    case EHTokRightBrace:
                        return false;

    default:
        {
                        if (acceptDeclaration(statement))
                return true;

                        TIntermTyped* node;
            if (acceptExpression(node))
                statement = node;
            else
                return false;

                        if (! acceptTokenClass(EHTokSemicolon)) {
                expected(";");
                return false;
            }
        }
    }

    return true;
}

void HlslGrammar::acceptAttributes()
{
            do {
                if (! acceptTokenClass(EHTokLeftBracket))
            return;

                if (peekTokenClass(EHTokIdentifier)) {
                        advanceToken();
        } else if (! peekTokenClass(EHTokRightBracket)) {
            expected("identifier");
            advanceToken();
        }

                if (acceptTokenClass(EHTokLeftParen)) {
            TIntermTyped* node;
            if (! acceptLiteral(node))
                expected("literal");
                        if (! acceptTokenClass(EHTokRightParen))
                expected(")");
        }

                if (acceptTokenClass(EHTokRightBracket))
            continue;

        expected("]");
        return;

    } while (true);
}

bool HlslGrammar::acceptSelectionStatement(TIntermNode*& statement)
{
    TSourceLoc loc = token.loc;

        if (! acceptTokenClass(EHTokIf))
        return false;

            parseContext.pushScope();

        TIntermTyped* condition;
    if (! acceptParenExpression(condition))
        return false;

        TIntermNodePair thenElse = { nullptr, nullptr };

        if (! acceptScopedStatement(thenElse.node1)) {
        expected("then statement");
        return false;
    }

        if (acceptTokenClass(EHTokElse)) {
                if (! acceptScopedStatement(thenElse.node2)) {
            expected("else statement");
            return false;
        }
    }

        statement = intermediate.addSelection(condition, thenElse, loc);
    parseContext.popScope();

    return true;
}

bool HlslGrammar::acceptSwitchStatement(TIntermNode*& statement)
{
        TSourceLoc loc = token.loc;
    if (! acceptTokenClass(EHTokSwitch))
        return false;

        parseContext.pushScope();
    TIntermTyped* switchExpression;
    if (! acceptParenExpression(switchExpression)) {
        parseContext.popScope();
        return false;
    }

        parseContext.pushSwitchSequence(new TIntermSequence);
    bool statementOkay = acceptCompoundStatement(statement);
    if (statementOkay)
        statement = parseContext.addSwitch(loc, switchExpression, statement ? statement->getAsAggregate() : nullptr);

    parseContext.popSwitchSequence();
    parseContext.popScope();

    return statementOkay;
}

bool HlslGrammar::acceptIterationStatement(TIntermNode*& statement)
{
    TSourceLoc loc = token.loc;
    TIntermTyped* condition = nullptr;

    EHlslTokenClass loop = peek();
    assert(loop == EHTokDo || loop == EHTokFor || loop == EHTokWhile);

        advanceToken();

    switch (loop) {
    case EHTokWhile:
                        parseContext.pushScope();
        parseContext.nestLooping();

                if (! acceptParenExpression(condition))
            return false;

                if (! acceptScopedStatement(statement)) {
            expected("while sub-statement");
            return false;
        }

        parseContext.unnestLooping();
        parseContext.popScope();

        statement = intermediate.addLoop(statement, condition, nullptr, true, loc);

        return true;

    case EHTokDo:
        parseContext.nestLooping();

        if (! acceptTokenClass(EHTokLeftBrace))
            expected("{");

                if (! peekTokenClass(EHTokRightBrace) && ! acceptScopedStatement(statement)) {
            expected("do sub-statement");
            return false;
        }

        if (! acceptTokenClass(EHTokRightBrace))
            expected("}");

                if (! acceptTokenClass(EHTokWhile)) {
            expected("while");
            return false;
        }

                TIntermTyped* condition;
        if (! acceptParenExpression(condition))
            return false;

        if (! acceptTokenClass(EHTokSemicolon))
            expected(";");

        parseContext.unnestLooping();

        statement = intermediate.addLoop(statement, condition, 0, false, loc);

        return true;

    case EHTokFor:
    {
                if (! acceptTokenClass(EHTokLeftParen))
            expected("(");

                        parseContext.pushScope();

                TIntermNode* initNode = nullptr;
        if (! acceptControlDeclaration(initNode)) {
            TIntermTyped* initExpr = nullptr;
            acceptExpression(initExpr);
            initNode = initExpr;
        }
                if (! acceptTokenClass(EHTokSemicolon))
            expected(";");

        parseContext.nestLooping();

                acceptExpression(condition);
        if (! acceptTokenClass(EHTokSemicolon))
            expected(";");

                TIntermTyped* iterator = nullptr;
        acceptExpression(iterator);
        if (! acceptTokenClass(EHTokRightParen))
            expected(")");

                if (! acceptScopedStatement(statement)) {
            expected("for sub-statement");
            return false;
        }

        statement = intermediate.addForLoop(statement, initNode, condition, iterator, true, loc);

        parseContext.popScope();
        parseContext.unnestLooping();

        return true;
    }

    default:
        return false;
    }
}

bool HlslGrammar::acceptJumpStatement(TIntermNode*& statement)
{
    EHlslTokenClass jump = peek();
    switch (jump) {
    case EHTokContinue:
    case EHTokBreak:
    case EHTokDiscard:
    case EHTokReturn:
        advanceToken();
        break;
    default:
                return false;
    }

    switch (jump) {
    case EHTokContinue:
        statement = intermediate.addBranch(EOpContinue, token.loc);
        break;
    case EHTokBreak:
        statement = intermediate.addBranch(EOpBreak, token.loc);
        break;
    case EHTokDiscard:
        statement = intermediate.addBranch(EOpKill, token.loc);
        break;

    case EHTokReturn:
    {
                TIntermTyped* node;
        if (acceptExpression(node)) {
                        statement = intermediate.addBranch(EOpReturn, node, token.loc);
        } else
            statement = intermediate.addBranch(EOpReturn, token.loc);
        break;
    }

    default:
        assert(0);
        return false;
    }

        if (! acceptTokenClass(EHTokSemicolon))
        expected(";");
    
    return true;
}

bool HlslGrammar::acceptCaseLabel(TIntermNode*& statement)
{
    TSourceLoc loc = token.loc;
    if (! acceptTokenClass(EHTokCase))
        return false;

    TIntermTyped* expression;
    if (! acceptExpression(expression)) {
        expected("case expression");
        return false;
    }

    if (! acceptTokenClass(EHTokColon)) {
        expected(":");
        return false;
    }

    statement = parseContext.intermediate.addBranch(EOpCase, expression, loc);

    return true;
}

bool HlslGrammar::acceptDefaultLabel(TIntermNode*& statement)
{
    TSourceLoc loc = token.loc;
    if (! acceptTokenClass(EHTokDefault))
        return false;

    if (! acceptTokenClass(EHTokColon)) {
        expected(":");
        return false;
    }

    statement = parseContext.intermediate.addBranch(EOpDefault, loc);

    return true;
}

void HlslGrammar::acceptArraySpecifier(TArraySizes*& arraySizes)
{
    arraySizes = nullptr;

    if (! acceptTokenClass(EHTokLeftBracket))
        return;

    TSourceLoc loc = token.loc;
    TIntermTyped* sizeExpr;
    if (! acceptAssignmentExpression(sizeExpr)) {
        expected("array-sizing expression");
        return;
    }

    if (! acceptTokenClass(EHTokRightBracket)) {
        expected("]");
        return;
    }

    TArraySize arraySize;
    parseContext.arraySizeCheck(loc, sizeExpr, arraySize);
    arraySizes = new TArraySizes;
    arraySizes->addInnerSize(arraySize);
}

void HlslGrammar::acceptPostDecls(TType& type)
{
    do {
                if (acceptTokenClass(EHTokColon)) {
            HlslToken idToken;
            if (acceptTokenClass(EHTokPackOffset)) {
                if (! acceptTokenClass(EHTokLeftParen)) {
                    expected("(");
                    return;
                }
                acceptTokenClass(EHTokIdentifier);
                acceptTokenClass(EHTokDot);
                acceptTokenClass(EHTokIdentifier);
                if (! acceptTokenClass(EHTokRightParen)) {
                    expected(")");
                    break;
                }
                            } else if (! acceptIdentifier(idToken)) {
                expected("semantic or packoffset or register");
                return;
            } else if (*idToken.string == "register") {
                if (! acceptTokenClass(EHTokLeftParen)) {
                    expected("(");
                    return;
                }
                acceptTokenClass(EHTokIdentifier);
                acceptTokenClass(EHTokComma);
                acceptTokenClass(EHTokIdentifier);
                acceptTokenClass(EHTokLeftBracket);
                if (peekTokenClass(EHTokIntConstant))
                    advanceToken();
                acceptTokenClass(EHTokRightBracket);
                if (! acceptTokenClass(EHTokRightParen)) {
                    expected(")");
                    break;
                }
                            } else {
                                parseContext.handleSemantic(type, *idToken.string);
            }
        } else if (acceptTokenClass(EHTokLeftAngle)) {
                        do {
                if (peekTokenClass(EHTokNone))
                    return;
                if (acceptTokenClass(EHTokRightAngle))
                    break;
                advanceToken();
            } while (true);
        } else
            break;

    } while (true);
}

} 