
package com.alibaba.fastjson.serializer;

import java.io.IOException;
import java.lang.reflect.Type;
import java.math.BigInteger;

import com.alibaba.fastjson.parser.DefaultJSONParser;
import com.alibaba.fastjson.parser.JSONLexer;
import com.alibaba.fastjson.parser.JSONToken;
import com.alibaba.fastjson.parser.deserializer.ObjectDeserializer;
import com.alibaba.fastjson.util.TypeUtils;


public class BigIntegerCodec implements ObjectSerializer, ObjectDeserializer {

    public final static BigIntegerCodec instance = new BigIntegerCodec();

    public void write(JSONSerializer serializer, Object object, Object fieldName, Type fieldType, int features) throws IOException {
        SerializeWriter out = serializer.out;

        if (object == null) {
            out.writeNull(SerializerFeature.WriteNullNumberAsZero);
            return;
        }
        
        BigInteger val = (BigInteger) object;
        out.write(val.toString());
    }

    @SuppressWarnings("unchecked")
    public <T> T deserialze(DefaultJSONParser parser, Type clazz, Object fieldName) {
        return (T) deserialze(parser);
    }

    @SuppressWarnings("unchecked")
    public static <T> T deserialze(DefaultJSONParser parser) {
        final JSONLexer lexer = parser.lexer;
        if (lexer.token() == JSONToken.LITERAL_INT) {
            String val = lexer.numberString();
            lexer.nextToken(JSONToken.COMMA);
            return (T) new BigInteger(val);
        }

        Object value = parser.parse();
        return value == null             ? null             : (T) TypeUtils.castToBigInteger(value);
    }

    public int getFastMatchToken() {
        return JSONToken.LITERAL_INT;
    }
}
