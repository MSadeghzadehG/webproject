
package com.alibaba.dubbo.common.json;

import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.Reader;
import java.io.UnsupportedEncodingException;


@Deprecated
public class JSONReader {
    private static ThreadLocal<Yylex> LOCAL_LEXER = new ThreadLocal<Yylex>() {
    };

    private Yylex mLex;

    public JSONReader(InputStream is, String charset) throws UnsupportedEncodingException {
        this(new InputStreamReader(is, charset));
    }

    public JSONReader(Reader reader) {
        mLex = getLexer(reader);
    }

    private static Yylex getLexer(Reader reader) {
        Yylex ret = LOCAL_LEXER.get();
        if (ret == null) {
            ret = new Yylex(reader);
            LOCAL_LEXER.set(ret);
        } else {
            ret.yyreset(reader);
        }
        return ret;
    }

    public JSONToken nextToken() throws IOException, ParseException {
        return mLex.yylex();
    }

    public JSONToken nextToken(int expect) throws IOException, ParseException {
        JSONToken ret = mLex.yylex();
        if (ret == null)
            throw new ParseException("EOF error.");
        if (expect != JSONToken.ANY && expect != ret.type)
            throw new ParseException("Unexpected token.");
        return ret;
    }
}