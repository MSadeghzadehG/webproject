

#define _CRT_SECURE_NO_WARNINGS

#include <stdarg.h>
#include <stdio.h>
#include <sstream>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "PpContext.h"
#include "PpTokens.h"

namespace glslang {

int TPpContext::InitCPP()
{
    pool = mem_CreatePool(0, 0);

    return 1;
}

int TPpContext::CPPdefine(TPpToken* ppToken)
{
    MacroSymbol mac;
    Symbol *symb;

        int token = scanToken(ppToken);
    if (token != PpAtomIdentifier) {
        parseContext.ppError(ppToken->loc, "must be followed by macro name", "#define", "");
        return token;
    }
    if (ppToken->loc.string >= 0) {
                parseContext.reservedPpErrorCheck(ppToken->loc, ppToken->name, "#define");
    }

        const int defAtom = ppToken->atom;

        token = scanToken(ppToken);
    if (token == '(' && ! ppToken->space) {
        int argc = 0;
        int args[maxMacroArgs];
        do {
            token = scanToken(ppToken);
            if (argc == 0 && token == ')') 
                break;
            if (token != PpAtomIdentifier) {
                parseContext.ppError(ppToken->loc, "bad argument", "#define", "");

                return token;
            }
                        bool duplicate = false;
            for (int a = 0; a < argc; ++a) {
                if (args[a] == ppToken->atom) {
                    parseContext.ppError(ppToken->loc, "duplicate macro parameter", "#define", "");
                    duplicate = true;
                    break;
                }
            }
            if (! duplicate) {
                if (argc < maxMacroArgs)
                    args[argc++] = ppToken->atom;
                else
                    parseContext.ppError(ppToken->loc, "too many macro parameters", "#define", "");
            }
            token = scanToken(ppToken);
        } while (token == ',');
        if (token != ')') {
            parseContext.ppError(ppToken->loc, "missing parenthesis", "#define", "");

            return token;
        }
        mac.argc = argc;
        mac.args = (int*)mem_Alloc(pool, argc * sizeof(int));
        memcpy(mac.args, args, argc * sizeof(int));
        token = scanToken(ppToken);
    }

        TSourceLoc defineLoc = ppToken->loc;     mac.body = new TokenStream;
    while (token != '\n' && token != EndOfInput) {
        RecordToken(mac.body, token, ppToken);
        token = scanToken(ppToken);
        if (token != '\n' && ppToken->space)
            RecordToken(mac.body, ' ', ppToken);
    }

        symb = LookUpSymbol(defAtom);
    if (symb) {
        if (! symb->mac.undef) {
                                                if (symb->mac.argc != mac.argc)
                parseContext.ppError(defineLoc, "Macro redefined; different number of arguments:", "#define", GetAtomString(defAtom));
            else {
                for (int argc = 0; argc < mac.argc; argc++) {
                    if (symb->mac.args[argc] != mac.args[argc])
                        parseContext.ppError(defineLoc, "Macro redefined; different argument names:", "#define", GetAtomString(defAtom));
                }
                RewindTokenStream(symb->mac.body);
                RewindTokenStream(mac.body);
                int newToken;
                do {
                    int oldToken;
                    TPpToken oldPpToken;
                    TPpToken newPpToken;                    
                    oldToken = ReadToken(symb->mac.body, &oldPpToken);
                    newToken = ReadToken(mac.body, &newPpToken);
                    if (oldToken != newToken || oldPpToken != newPpToken) {
                        parseContext.ppError(defineLoc, "Macro redefined; different substitutions:", "#define", GetAtomString(defAtom));
                        break; 
                    }
                } while (newToken > 0);
            }
        }
    } else
        symb = AddSymbol(defAtom);

    delete symb->mac.body;
    symb->mac = mac;

    return '\n';
}

int TPpContext::CPPundef(TPpToken* ppToken)
{
    int token = scanToken(ppToken);
    Symbol *symb;
    if (token != PpAtomIdentifier) {
        parseContext.ppError(ppToken->loc, "must be followed by macro name", "#undef", "");

        return token;
    }

    parseContext.reservedPpErrorCheck(ppToken->loc, ppToken->name, "#undef");

    symb = LookUpSymbol(ppToken->atom);
    if (symb) {
        symb->mac.undef = 1;
    }
    token = scanToken(ppToken);
    if (token != '\n')
        parseContext.ppError(ppToken->loc, "can only be followed by a single macro name", "#undef", "");

    return token;
}


int TPpContext::CPPelse(int matchelse, TPpToken* ppToken)
{
    int atom;
    int depth = 0;
    int token = scanToken(ppToken);

    while (token != EndOfInput) {
        if (token != '#') {
            while (token != '\n' && token != EndOfInput)
                token = scanToken(ppToken);
            
            if (token == EndOfInput)
                return token;

            token = scanToken(ppToken);
            continue;
        }

        if ((token = scanToken(ppToken)) != PpAtomIdentifier)
            continue;

        atom = ppToken->atom;
        if (atom == PpAtomIf || atom == PpAtomIfdef || atom == PpAtomIfndef) {
            depth++; 
            ifdepth++; 
            elsetracker++;
        } else if (atom == PpAtomEndif) {
            token = extraTokenCheck(atom, ppToken, scanToken(ppToken));
            elseSeen[elsetracker] = false;
            --elsetracker;
            if (depth == 0) {
                                if (ifdepth) 
                    --ifdepth;
                break;
            }
            --depth;
            --ifdepth;
        } else if (matchelse && depth == 0) {
            if (atom == PpAtomElse) {
                elseSeen[elsetracker] = true;
                token = extraTokenCheck(atom, ppToken, scanToken(ppToken));
                                break;
            } else if (atom == PpAtomElif) {
                if (elseSeen[elsetracker])
                    parseContext.ppError(ppToken->loc, "#elif after #else", "#elif", "");
                
                if (ifdepth) {
                    --ifdepth;
                    elseSeen[elsetracker] = false;
                    --elsetracker;
                }

                return CPPif(ppToken);
            }
        } else if (atom == PpAtomElse) {
            if (elseSeen[elsetracker])
                parseContext.ppError(ppToken->loc, "#else after #else", "#else", "");
            else
                elseSeen[elsetracker] = true;
            token = extraTokenCheck(atom, ppToken, scanToken(ppToken));
        } else if (atom == PpAtomElif) {
            if (elseSeen[elsetracker])
                parseContext.ppError(ppToken->loc, "#elif after #else", "#elif", "");
        }
    }

    return token;
}

int TPpContext::extraTokenCheck(int atom, TPpToken* ppToken, int token)
{
    if (token != '\n' && token != EndOfInput) {
        static const char* message = "unexpected tokens following directive";

        const char* label;
        if (atom == PpAtomElse)
            label = "#else";
        else if (atom == PpAtomElif)
            label = "#elif";
        else if (atom == PpAtomEndif)
            label = "#endif";
        else if (atom == PpAtomIf)
            label = "#if";
        else if (atom == PpAtomLine)
            label = "#line";
        else
            label = "";

        if (parseContext.relaxedErrors())
            parseContext.ppWarn(ppToken->loc, message, label, "");
        else
            parseContext.ppError(ppToken->loc, message, label, "");

        while (token != '\n' && token != EndOfInput)
            token = scanToken(ppToken);
    }

    return token;
}

enum eval_prec {
    MIN_PRECEDENCE,
    COND, LOGOR, LOGAND, OR, XOR, AND, EQUAL, RELATION, SHIFT, ADD, MUL, UNARY,
    MAX_PRECEDENCE
};

namespace {

    int op_logor(int a, int b) { return a || b; }
    int op_logand(int a, int b) { return a && b; }
    int op_or(int a, int b) { return a | b; }
    int op_xor(int a, int b) { return a ^ b; }
    int op_and(int a, int b) { return a & b; }
    int op_eq(int a, int b) { return a == b; }
    int op_ne(int a, int b) { return a != b; }
    int op_ge(int a, int b) { return a >= b; }
    int op_le(int a, int b) { return a <= b; }
    int op_gt(int a, int b) { return a > b; }
    int op_lt(int a, int b) { return a < b; }
    int op_shl(int a, int b) { return a << b; }
    int op_shr(int a, int b) { return a >> b; }
    int op_add(int a, int b) { return a + b; }
    int op_sub(int a, int b) { return a - b; }
    int op_mul(int a, int b) { return a * b; }
    int op_div(int a, int b) { return a / b; }
    int op_mod(int a, int b) { return a % b; }
    int op_pos(int a) { return a; }
    int op_neg(int a) { return -a; }
    int op_cmpl(int a) { return ~a; }
    int op_not(int a) { return !a; }

};

struct TBinop {
    int token, precedence, (*op)(int, int);
} binop[] = {
    { PpAtomOr, LOGOR, op_logor },
    { PpAtomAnd, LOGAND, op_logand },
    { '|', OR, op_or },
    { '^', XOR, op_xor },
    { '&', AND, op_and },
    { PpAtomEQ, EQUAL, op_eq },
    { PpAtomNE, EQUAL, op_ne },
    { '>', RELATION, op_gt },
    { PpAtomGE, RELATION, op_ge },
    { '<', RELATION, op_lt },
    { PpAtomLE, RELATION, op_le },
    { PpAtomLeft, SHIFT, op_shl },
    { PpAtomRight, SHIFT, op_shr },
    { '+', ADD, op_add },
    { '-', ADD, op_sub },
    { '*', MUL, op_mul },
    { '/', MUL, op_div },
    { '%', MUL, op_mod },
};

struct TUnop {
    int token, (*op)(int);
} unop[] = {
    { '+', op_pos },
    { '-', op_neg },
    { '~', op_cmpl },
    { '!', op_not },
};

#define NUM_ELEMENTS(A) (sizeof(A) / sizeof(A[0]))

int TPpContext::eval(int token, int precedence, bool shortCircuit, int& res, bool& err, TPpToken* ppToken)
{
    TSourceLoc loc = ppToken->loc;      if (token == PpAtomIdentifier) {
        if (ppToken->atom == PpAtomDefined) {
            bool needclose = 0;
            token = scanToken(ppToken);
            if (token == '(') {
                needclose = true;
                token = scanToken(ppToken);
            }
            if (token != PpAtomIdentifier) {
                parseContext.ppError(loc, "incorrect directive, expected identifier", "preprocessor evaluation", "");
                err = true;
                res = 0;

                return token;
            }
            Symbol* s = LookUpSymbol(ppToken->atom);
            res = s ? ! s->mac.undef : 0;
            token = scanToken(ppToken);
            if (needclose) {
                if (token != ')') {
                    parseContext.ppError(loc, "expected ')'", "preprocessor evaluation", "");
                    err = true;
                    res = 0;

                    return token;
                }
                token = scanToken(ppToken);
            }
        } else {
            token = evalToToken(token, shortCircuit, res, err, ppToken);
            return eval(token, precedence, shortCircuit, res, err, ppToken);
        }
    } else if (token == PpAtomConstInt) {
        res = ppToken->ival;
        token = scanToken(ppToken);
    } else if (token == '(') {
        token = scanToken(ppToken);
        token = eval(token, MIN_PRECEDENCE, shortCircuit, res, err, ppToken);
        if (! err) {
            if (token != ')') {
                parseContext.ppError(loc, "expected ')'", "preprocessor evaluation", "");
                err = true;
                res = 0;

                return token;
            }
            token = scanToken(ppToken);
        }
    } else {
        int op = NUM_ELEMENTS(unop) - 1;
        for (; op >= 0; op--) {
            if (unop[op].token == token)
                break;
        }
        if (op >= 0) {
            token = scanToken(ppToken);
            token = eval(token, UNARY, shortCircuit, res, err, ppToken);
            res = unop[op].op(res);
        } else {
            parseContext.ppError(loc, "bad expression", "preprocessor evaluation", "");
            err = true;
            res = 0;

            return token;
        }
    }

    token = evalToToken(token, shortCircuit, res, err, ppToken);

        while (! err) {
        if (token == ')' || token == '\n') 
            break;
        int op;
        for (op = NUM_ELEMENTS(binop) - 1; op >= 0; op--) {
            if (binop[op].token == token)
                break;
        }
        if (op < 0 || binop[op].precedence <= precedence)
            break;
        int leftSide = res;
        
                        if (! shortCircuit) {
            if ((token == PpAtomOr  && leftSide == 1) ||
                (token == PpAtomAnd && leftSide == 0))
                shortCircuit = true;
        }

        token = scanToken(ppToken);
        token = eval(token, binop[op].precedence, shortCircuit, res, err, ppToken);

        if (binop[op].op == op_div || binop[op].op == op_mod) {
            if (res == 0) {
                parseContext.ppError(loc, "division by 0", "preprocessor evaluation", "");
                res = 1;
            }
        }
        res = binop[op].op(leftSide, res);
    }

    return token;
}

int TPpContext::evalToToken(int token, bool shortCircuit, int& res, bool& err, TPpToken* ppToken)
{
    while (token == PpAtomIdentifier && ppToken->atom != PpAtomDefined) {
        int macroReturn = MacroExpand(ppToken->atom, ppToken, true, false);
        if (macroReturn == 0) {
            parseContext.ppError(ppToken->loc, "can't evaluate expression", "preprocessor evaluation", "");
            err = true;
            res = 0;
            token = scanToken(ppToken);
            break;
        }
        if (macroReturn == -1) {
            if (! shortCircuit && parseContext.profile == EEsProfile) {
                const char* message = "undefined macro in expression not allowed in es profile";
                if (parseContext.relaxedErrors())
                    parseContext.ppWarn(ppToken->loc, message, "preprocessor evaluation", ppToken->name);
                else
                    parseContext.ppError(ppToken->loc, message, "preprocessor evaluation", ppToken->name);
            }
        }
        token = scanToken(ppToken);
    }

    return token;
}

int TPpContext::CPPif(TPpToken* ppToken) 
{
    int token = scanToken(ppToken);
    elsetracker++;
    ifdepth++;
    if (ifdepth > maxIfNesting) {
        parseContext.ppError(ppToken->loc, "maximum nesting depth exceeded", "#if", "");
        return 0;
    }
    int res = 0;
    bool err = false;
    token = eval(token, MIN_PRECEDENCE, false, res, err, ppToken);
    token = extraTokenCheck(PpAtomIf, ppToken, token);
    if (!res && !err)
        token = CPPelse(1, ppToken);

    return token;
}

int TPpContext::CPPifdef(int defined, TPpToken* ppToken)
{
    int token = scanToken(ppToken);
    int name = ppToken->atom;
    if (++ifdepth > maxIfNesting) {
        parseContext.ppError(ppToken->loc, "maximum nesting depth exceeded", "#ifdef", "");
        return 0;
    }
    elsetracker++;
    if (token != PpAtomIdentifier) {
        if (defined)
            parseContext.ppError(ppToken->loc, "must be followed by macro name", "#ifdef", "");
        else 
            parseContext.ppError(ppToken->loc, "must be followed by macro name", "#ifndef", "");
    } else {
        Symbol *s = LookUpSymbol(name);
        token = scanToken(ppToken);
        if (token != '\n') {
            parseContext.ppError(ppToken->loc, "unexpected tokens following #ifdef directive - expected a newline", "#ifdef", "");
            while (token != '\n' && token != EndOfInput)
                token = scanToken(ppToken);
        }
        if (((s && !s->mac.undef) ? 1 : 0) != defined)
            token = CPPelse(1, ppToken);
    }

    return token;
}

int TPpContext::CPPinclude(TPpToken* ppToken)
{
    const TSourceLoc directiveLoc = ppToken->loc;
    int token = scanToken(ppToken);
    if (token != PpAtomConstString) {
                parseContext.ppError(directiveLoc, "must be followed by a file designation", "#include", "");
    } else {
                const std::string filename = ppToken->name;
        token = scanToken(ppToken);
        if (token != '\n' && token != EndOfInput) {
            parseContext.ppError(ppToken->loc, "extra content after file designation", "#include", "");
        } else {
            TShader::Includer::IncludeResult* res = includer.include(filename.c_str(), TShader::Includer::EIncludeRelative, currentSourceFile.c_str(), includeStack.size() + 1);
            if (res && !res->file_name.empty()) {
                if (res->file_data && res->file_length) {
                    const bool forNextLine = parseContext.lineDirectiveShouldSetNextLine();
                    std::ostringstream prologue;
                    std::ostringstream epilogue;
                    prologue << "#line " << forNextLine << " " << "\"" << res->file_name << "\"\n";
                    epilogue << (res->file_data[res->file_length - 1] == '\n'? "" : "\n") << "#line " << directiveLoc.line + forNextLine << " " << directiveLoc.getStringNameOrNum() << "\n";
                    pushInput(new TokenizableIncludeFile(directiveLoc, prologue.str(), res, epilogue.str(), this));
                }
                                if (token != EndOfInput) parseContext.setCurrentColumn(0);
                                return '\n';
            } else {
                std::string message =
                    res ? std::string(res->file_data, res->file_length)
                        : std::string("Could not process include directive");
                parseContext.ppError(directiveLoc, message.c_str(), "#include", "");
                if (res) {
                    includer.releaseInclude(res);
                }
            }
        }
    }
    return token;
}

int TPpContext::CPPline(TPpToken* ppToken) 
{
            
    int token = scanToken(ppToken);
    const TSourceLoc directiveLoc = ppToken->loc;
    if (token == '\n') {
        parseContext.ppError(ppToken->loc, "must by followed by an integral literal", "#line", "");
        return token;
    }

    int lineRes = 0;     int lineToken = 0;
    bool hasFile = false;
    int fileRes = 0;     const char* sourceName = nullptr;     bool lineErr = false;
    bool fileErr = false;
    token = eval(token, MIN_PRECEDENCE, false, lineRes, lineErr, ppToken);
    if (! lineErr) {
        lineToken = lineRes;
        if (token == '\n')
            ++lineRes;

        if (parseContext.lineDirectiveShouldSetNextLine())
            --lineRes;
        parseContext.setCurrentLine(lineRes);

        if (token != '\n') {
            if (token == PpAtomConstString) {
                parseContext.ppRequireExtensions(directiveLoc, 1, &E_GL_GOOGLE_cpp_style_line_directive, "filename-based #line");
                                                                sourceName = GetAtomString(LookUpAddString(ppToken->name));
                parseContext.setCurrentSourceName(sourceName);
                hasFile = true;
                token = scanToken(ppToken);
            } else {
                token = eval(token, MIN_PRECEDENCE, false, fileRes, fileErr, ppToken);
                if (! fileErr) {
                    parseContext.setCurrentString(fileRes);
                    hasFile = true;
                }
            }
        }
    }
    if (!fileErr && !lineErr) {
        parseContext.notifyLineDirective(directiveLoc.line, lineToken, hasFile, fileRes, sourceName);
    }
    token = extraTokenCheck(PpAtomLine, ppToken, token);

    return token;
}

int TPpContext::CPPerror(TPpToken* ppToken) 
{
    int token = scanToken(ppToken);
    std::string message;
    TSourceLoc loc = ppToken->loc;

    while (token != '\n' && token != EndOfInput) {
        if (token == PpAtomConstInt   || token == PpAtomConstUint   ||
            token == PpAtomConstInt64 || token == PpAtomConstUint64 ||
            token == PpAtomConstFloat || token == PpAtomConstDouble) {
                message.append(ppToken->name);
        } else if (token == PpAtomIdentifier || token == PpAtomConstString) {
            message.append(ppToken->name);
        } else {
            message.append(GetAtomString(token));
        }
        message.append(" ");
        token = scanToken(ppToken);
    }
    parseContext.notifyErrorDirective(loc.line, message.c_str());
        parseContext.ppError(loc, message.c_str(), "#error", "");

    return '\n';
}

int TPpContext::CPPpragma(TPpToken* ppToken)
{
    char SrcStrName[2];
    TVector<TString> tokens;

    TSourceLoc loc = ppToken->loc;      int token = scanToken(ppToken);
    while (token != '\n' && token != EndOfInput) {
        switch (token) {
        case PpAtomIdentifier:
        case PpAtomConstInt:
        case PpAtomConstUint:
        case PpAtomConstInt64:
        case PpAtomConstUint64:
        case PpAtomConstFloat:
        case PpAtomConstDouble:
            tokens.push_back(ppToken->name);
            break;
        default:
            SrcStrName[0] = (char)token;
            SrcStrName[1] = '\0';
            tokens.push_back(SrcStrName);
        }
        token = scanToken(ppToken);
    }

    if (token == EndOfInput)
        parseContext.ppError(loc, "directive must end with a newline", "#pragma", "");
    else
        parseContext.handlePragma(loc, tokens);

    return token;    
}

int TPpContext::CPPversion(TPpToken* ppToken)
{
    int token = scanToken(ppToken);

    if (errorOnVersion || versionSeen)
        parseContext.ppError(ppToken->loc, "must occur first in shader", "#version", "");
    versionSeen = true;

    if (token == '\n') {
        parseContext.ppError(ppToken->loc, "must be followed by version number", "#version", "");

        return token;
    }

    if (token != PpAtomConstInt)
        parseContext.ppError(ppToken->loc, "must be followed by version number", "#version", "");

    ppToken->ival = atoi(ppToken->name);
    int versionNumber = ppToken->ival;
    int line = ppToken->loc.line;
    token = scanToken(ppToken);

    if (token == '\n') {
        parseContext.notifyVersion(line, versionNumber, nullptr);
        return token;
    } else {
        if (ppToken->atom != PpAtomCore &&
            ppToken->atom != PpAtomCompatibility &&
            ppToken->atom != PpAtomEs)
            parseContext.ppError(ppToken->loc, "bad profile name; use es, core, or compatibility", "#version", "");
        parseContext.notifyVersion(line, versionNumber, ppToken->name);
        token = scanToken(ppToken);

        if (token == '\n')
            return token;
        else
            parseContext.ppError(ppToken->loc, "bad tokens following profile -- expected newline", "#version", "");
    }

    return token;
}

int TPpContext::CPPextension(TPpToken* ppToken)
{
    int line = ppToken->loc.line;
    int token = scanToken(ppToken);
    char extensionName[MaxTokenLength + 1];

    if (token=='\n') {
        parseContext.ppError(ppToken->loc, "extension name not specified", "#extension", "");
        return token;
    }

    if (token != PpAtomIdentifier)
        parseContext.ppError(ppToken->loc, "extension name expected", "#extension", "");

    assert(strlen(ppToken->name) <= MaxTokenLength);
    strcpy(extensionName, ppToken->name);

    token = scanToken(ppToken);
    if (token != ':') {
        parseContext.ppError(ppToken->loc, "':' missing after extension name", "#extension", "");
        return token;
    }

    token = scanToken(ppToken);
    if (token != PpAtomIdentifier) {
        parseContext.ppError(ppToken->loc, "behavior for extension not specified", "#extension", "");
        return token;
    }

    parseContext.updateExtensionBehavior(line, extensionName, ppToken->name);
    parseContext.notifyExtensionDirective(line, extensionName, ppToken->name);

    token = scanToken(ppToken);
    if (token == '\n')
        return token;
    else
        parseContext.ppError(ppToken->loc,  "extra tokens -- expected newline", "#extension","");

    return token;
}

int TPpContext::readCPPline(TPpToken* ppToken)
{
    int token = scanToken(ppToken);

    if (token == PpAtomIdentifier) {
        switch (ppToken->atom) {
        case PpAtomDefine:
            token = CPPdefine(ppToken);
            break;
        case PpAtomElse:
            if (elsetracker[elseSeen])
                parseContext.ppError(ppToken->loc, "#else after #else", "#else", "");
            elsetracker[elseSeen] = true;
            if (! ifdepth)
                parseContext.ppError(ppToken->loc, "mismatched statements", "#else", "");
            token = extraTokenCheck(PpAtomElse, ppToken, scanToken(ppToken));
            token = CPPelse(0, ppToken);
            break;
        case PpAtomElif:
            if (! ifdepth)
                parseContext.ppError(ppToken->loc, "mismatched statements", "#elif", "");
            if (elseSeen[elsetracker])
                parseContext.ppError(ppToken->loc, "#elif after #else", "#elif", "");
                        token = scanToken(ppToken); 
            while (token != '\n' && token != EndOfInput)
                token = scanToken(ppToken);
            token = CPPelse(0, ppToken);
            break;
        case PpAtomEndif:
            if (! ifdepth)
                parseContext.ppError(ppToken->loc, "mismatched statements", "#endif", "");
            else {
                elseSeen[elsetracker] = false;
                --elsetracker;
                --ifdepth;
            }
            token = extraTokenCheck(PpAtomEndif, ppToken, scanToken(ppToken));
            break;
        case PpAtomIf:
            token = CPPif(ppToken);
            break;
        case PpAtomIfdef:
            token = CPPifdef(1, ppToken);
            break;
        case PpAtomIfndef:
            token = CPPifdef(0, ppToken);
            break;
        case PpAtomInclude:
            parseContext.ppRequireExtensions(ppToken->loc, 1, &E_GL_GOOGLE_include_directive, "#include");
            token = CPPinclude(ppToken);
            break;
        case PpAtomLine:
            token = CPPline(ppToken);
            break;
        case PpAtomPragma:
            token = CPPpragma(ppToken);
            break;
        case PpAtomUndef:
            token = CPPundef(ppToken);
            break;
        case PpAtomError:
            token = CPPerror(ppToken);
            break;
        case PpAtomVersion:
            token = CPPversion(ppToken);
            break;
        case PpAtomExtension:
            token = CPPextension(ppToken);
            break;
        default:
            parseContext.ppError(ppToken->loc, "invalid directive:", "#", ppToken->name);
            break;
        }
    } else if (token != '\n' && token != EndOfInput)
        parseContext.ppError(ppToken->loc, "invalid directive", "#", "");

    while (token != '\n' && token != EndOfInput)
        token = scanToken(ppToken);

    return token;
}

TPpContext::TokenStream* TPpContext::PrescanMacroArg(TokenStream* a, TPpToken* ppToken, bool newLineOkay)
{
    int token;
    TokenStream *n;
    RewindTokenStream(a);
    do {
        token = ReadToken(a, ppToken);
        if (token == PpAtomIdentifier && LookUpSymbol(ppToken->atom))
            break;
    } while (token != EndOfInput);

    if (token == EndOfInput)
        return a;

    n = new TokenStream;
    pushInput(new tMarkerInput(this));
    pushTokenStreamInput(a);
    while ((token = scanToken(ppToken)) != tMarkerInput::marker) {
        if (token == PpAtomIdentifier && MacroExpand(ppToken->atom, ppToken, false, newLineOkay) != 0)
            continue;
        RecordToken(n, token, ppToken);
    }
    popInput();
    delete a;

    return n;
}

int TPpContext::tMacroInput::scan(TPpToken* ppToken)
{
    int token;
    do {
        token = pp->ReadToken(mac->body, ppToken);
    } while (token == ' ');  
        if (token == PpAtomIdentifier) {
        int i;
        for (i = mac->argc - 1; i >= 0; i--)
            if (mac->args[i] == ppToken->atom) 
                break;
        if (i >= 0) {
            pp->pushTokenStreamInput(args[i]);

            return pp->scanToken(ppToken);
        }
    }

    if (token == EndOfInput)
        mac->busy = 0;
        
    return token;
}

int TPpContext::tZeroInput::scan(TPpToken* ppToken)
{
    if (done)
        return EndOfInput;

    strcpy(ppToken->name, "0");
    ppToken->ival = 0;
    ppToken->space = false;
    done = true;

    return PpAtomConstInt;
}

int TPpContext::MacroExpand(int atom, TPpToken* ppToken, bool expandUndef, bool newLineOkay)
{
    ppToken->space = false;
    switch (atom) {
    case PpAtomLineMacro:
        ppToken->ival = parseContext.getCurrentLoc().line;
        snprintf(ppToken->name, sizeof(ppToken->name), "%d", ppToken->ival);
        UngetToken(PpAtomConstInt, ppToken);
        return 1;

    case PpAtomFileMacro: {
        if (parseContext.getCurrentLoc().name)
            parseContext.ppRequireExtensions(ppToken->loc, 1, &E_GL_GOOGLE_cpp_style_line_directive, "filename-based __FILE__");
        ppToken->ival = parseContext.getCurrentLoc().string;
        snprintf(ppToken->name, sizeof(ppToken->name), "%s", ppToken->loc.getStringNameOrNum().c_str());
        UngetToken(PpAtomConstInt, ppToken);
        return 1;
    }

    case PpAtomVersionMacro:
        ppToken->ival = parseContext.version;
        snprintf(ppToken->name, sizeof(ppToken->name), "%d", ppToken->ival);
        UngetToken(PpAtomConstInt, ppToken);
        return 1;

    default:
        break;
    }

    Symbol *sym = LookUpSymbol(atom);
    int token;
    int depth = 0;

        if (sym && sym->mac.busy)
        return 0;

        if ((! sym || sym->mac.undef) && ! expandUndef)
        return 0;

        if ((! sym || sym->mac.undef) && expandUndef) {
        pushInput(new tZeroInput(this));
        return -1;
    }

    tMacroInput *in = new tMacroInput(this);

    TSourceLoc loc = ppToken->loc;      in->mac = &sym->mac;
    if (sym->mac.args) {
        token = scanToken(ppToken);
        if (newLineOkay) {
            while (token == '\n')                
                token = scanToken(ppToken);
        }
        if (token != '(') {
            parseContext.ppError(loc, "expected '(' following", "macro expansion", GetAtomString(atom));
            UngetToken(token, ppToken);
            ppToken->atom = atom;

            delete in;
            return 0;
        }
        in->args.resize(in->mac->argc);
        for (int i = 0; i < in->mac->argc; i++)
            in->args[i] = new TokenStream;
        int arg = 0;
        bool tokenRecorded = false;
        do {
            depth = 0;
            while (1) {
                token = scanToken(ppToken);
                if (token == EndOfInput) {
                    parseContext.ppError(loc, "End of input in macro", "macro expansion", GetAtomString(atom));
                    delete in;
                    return 0;
                }
                if (token == '\n') {
                    if (! newLineOkay) {
                        parseContext.ppError(loc, "End of line in macro substitution:", "macro expansion", GetAtomString(atom));
                        delete in;
                        return 0;
                    }
                    continue;
                }
                if (token == '#') {
                    parseContext.ppError(ppToken->loc, "unexpected '#'", "macro expansion", GetAtomString(atom));
                    delete in;
                    return 0;
                }
                if (in->mac->argc == 0 && token != ')')
                    break;
                if (depth == 0 && (token == ',' || token == ')'))
                    break;
                if (token == '(')
                    depth++;
                if (token == ')')
                    depth--;
                RecordToken(in->args[arg], token, ppToken);
                tokenRecorded = true;
            }
            if (token == ')') {
                if (in->mac->argc == 1 && tokenRecorded == 0)
                    break;
                arg++;
                break;
            }
            arg++;
        } while (arg < in->mac->argc);

        if (arg < in->mac->argc)
            parseContext.ppError(loc, "Too few args in Macro", "macro expansion", GetAtomString(atom));
        else if (token != ')') {
            depth=0;
            while (token != EndOfInput && (depth > 0 || token != ')')) {
                if (token == ')')
                    depth--;
                token = scanToken(ppToken);
                if (token == '(')
                    depth++;
            }

            if (token == EndOfInput) {
                parseContext.ppError(loc, "End of input in macro", "macro expansion", GetAtomString(atom));
                delete in;
                return 0;
            }
            parseContext.ppError(loc, "Too many args in macro", "macro expansion", GetAtomString(atom));
        }
        for (int i = 0; i < in->mac->argc; i++)
            in->args[i] = PrescanMacroArg(in->args[i], ppToken, newLineOkay);
    }

    pushInput(in);
    sym->mac.busy = 1;
    RewindTokenStream(sym->mac.body);

    return 1;
}

} 