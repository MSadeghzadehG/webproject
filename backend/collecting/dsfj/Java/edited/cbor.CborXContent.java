

package org.elasticsearch.common.xcontent.cbor;

import com.fasterxml.jackson.core.JsonEncoding;
import com.fasterxml.jackson.core.JsonGenerator;
import com.fasterxml.jackson.core.JsonParser;
import com.fasterxml.jackson.dataformat.cbor.CBORFactory;
import org.elasticsearch.ElasticsearchParseException;
import org.elasticsearch.common.xcontent.DeprecationHandler;
import org.elasticsearch.common.xcontent.NamedXContentRegistry;
import org.elasticsearch.common.xcontent.XContent;
import org.elasticsearch.common.xcontent.XContentBuilder;
import org.elasticsearch.common.xcontent.XContentGenerator;
import org.elasticsearch.common.xcontent.XContentParser;
import org.elasticsearch.common.xcontent.XContentType;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.Reader;
import java.io.StringReader;
import java.util.Set;


public class CborXContent implements XContent {

    public static XContentBuilder contentBuilder() throws IOException {
        return XContentBuilder.builder(cborXContent);
    }

    static final CBORFactory cborFactory;
    public static final CborXContent cborXContent;

    static {
        cborFactory = new CBORFactory();
        cborFactory.configure(CBORFactory.Feature.FAIL_ON_SYMBOL_HASH_OVERFLOW, false);                 cborFactory.configure(JsonGenerator.Feature.AUTO_CLOSE_JSON_CONTENT, false);
        cborFactory.configure(JsonParser.Feature.STRICT_DUPLICATE_DETECTION, XContent.isStrictDuplicateDetectionEnabled());
        cborXContent = new CborXContent();
    }

    private CborXContent() {
    }

    @Override
    public XContentType type() {
        return XContentType.CBOR;
    }

    @Override
    public byte streamSeparator() {
        throw new ElasticsearchParseException("cbor does not support stream parsing...");
    }

    @Override
    public XContentGenerator createGenerator(OutputStream os, Set<String> includes, Set<String> excludes) throws IOException {
        return new CborXContentGenerator(cborFactory.createGenerator(os, JsonEncoding.UTF8), os, includes, excludes);
    }

    @Override
    public XContentParser createParser(NamedXContentRegistry xContentRegistry,
            DeprecationHandler deprecationHandler, String content) throws IOException {
        return new CborXContentParser(xContentRegistry, deprecationHandler, cborFactory.createParser(new StringReader(content)));
    }

    @Override
    public XContentParser createParser(NamedXContentRegistry xContentRegistry,
            DeprecationHandler deprecationHandler, InputStream is) throws IOException {
        return new CborXContentParser(xContentRegistry, deprecationHandler, cborFactory.createParser(is));
    }

    @Override
    public XContentParser createParser(NamedXContentRegistry xContentRegistry,
            DeprecationHandler deprecationHandler, byte[] data) throws IOException {
        return new CborXContentParser(xContentRegistry, deprecationHandler, cborFactory.createParser(data));
    }

    @Override
    public XContentParser createParser(NamedXContentRegistry xContentRegistry,
            DeprecationHandler deprecationHandler, byte[] data, int offset, int length) throws IOException {
        return new CborXContentParser(xContentRegistry, deprecationHandler, cborFactory.createParser(data, offset, length));
    }

    @Override
    public XContentParser createParser(NamedXContentRegistry xContentRegistry,
            DeprecationHandler deprecationHandler, Reader reader) throws IOException {
        return new CborXContentParser(xContentRegistry, deprecationHandler, cborFactory.createParser(reader));
    }

}
