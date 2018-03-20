
#ifndef HLSLGRAMMAR_H_
#define HLSLGRAMMAR_H_

#include "hlslParseHelper.h"
#include "hlslOpMap.h"
#include "hlslTokenStream.h"

namespace glslang {

        
    class HlslGrammar : public HlslTokenStream {
    public:
        HlslGrammar(HlslScanContext& scanner, HlslParseContext& parseContext)
            : HlslTokenStream(scanner), parseContext(parseContext), intermediate(parseContext.intermediate) { }
        virtual ~HlslGrammar() { }

        bool parse();

    protected:
        HlslGrammar();
        HlslGrammar& operator=(const HlslGrammar&);

        void expected(const char*);
        void unimplemented(const char*);
        bool acceptIdentifier(HlslToken&);
        bool acceptCompilationUnit();
        bool acceptDeclaration(TIntermNode*& node);
        bool acceptControlDeclaration(TIntermNode*& node);
        bool acceptSamplerDeclarationDX9(TType&);
        bool acceptSamplerState();
        bool acceptFullySpecifiedType(TType&);
        void acceptQualifier(TQualifier&);
        bool acceptType(TType&);
        bool acceptTemplateType(TBasicType&);
        bool acceptVectorTemplateType(TType&);
        bool acceptMatrixTemplateType(TType&);
        bool acceptSamplerType(TType&);
        bool acceptTextureType(TType&);
        bool acceptStruct(TType&);
        bool acceptStructDeclarationList(TTypeList*&);
        bool acceptFunctionParameters(TFunction&);
        bool acceptParameterDeclaration(TFunction&);
        bool acceptFunctionDefinition(TFunction&, TIntermNode*&);
        bool acceptParenExpression(TIntermTyped*&);
        bool acceptExpression(TIntermTyped*&);
        bool acceptInitializer(TIntermTyped*&);
        bool acceptAssignmentExpression(TIntermTyped*&);
        bool acceptBinaryExpression(TIntermTyped*&, PrecedenceLevel);
        bool acceptUnaryExpression(TIntermTyped*&);
        bool acceptPostfixExpression(TIntermTyped*&);
        bool acceptConstructor(TIntermTyped*&);
        bool acceptFunctionCall(HlslToken, TIntermTyped*&, TIntermTyped* base = nullptr);
        bool acceptArguments(TFunction*, TIntermTyped*&);
        bool acceptLiteral(TIntermTyped*&);
        bool acceptCompoundStatement(TIntermNode*&);
        bool acceptStatement(TIntermNode*&);
        bool acceptScopedStatement(TIntermNode*&);
        bool acceptScopedCompoundStatement(TIntermNode*&);
        bool acceptNestedStatement(TIntermNode*&);
        void acceptAttributes();
        bool acceptSelectionStatement(TIntermNode*&);
        bool acceptSwitchStatement(TIntermNode*&);
        bool acceptIterationStatement(TIntermNode*&);
        bool acceptJumpStatement(TIntermNode*&);
        bool acceptCaseLabel(TIntermNode*&);
        bool acceptDefaultLabel(TIntermNode*&);
        void acceptArraySpecifier(TArraySizes*&);
        void acceptPostDecls(TType&);

        HlslParseContext& parseContext;          TIntermediate& intermediate;         };

} 
#endif 