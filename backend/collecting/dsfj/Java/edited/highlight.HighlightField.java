

package org.elasticsearch.search.fetch.subphase.highlight;

import org.elasticsearch.common.ParsingException;
import org.elasticsearch.common.io.stream.StreamInput;
import org.elasticsearch.common.io.stream.StreamOutput;
import org.elasticsearch.common.io.stream.Streamable;
import org.elasticsearch.common.text.Text;
import org.elasticsearch.common.xcontent.ToXContentFragment;
import org.elasticsearch.common.xcontent.XContentBuilder;
import org.elasticsearch.common.xcontent.XContentParser;

import java.io.IOException;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.Objects;

import static org.elasticsearch.common.xcontent.XContentParserUtils.ensureExpectedToken;


public class HighlightField implements ToXContentFragment, Streamable {

    private String name;

    private Text[] fragments;

    HighlightField() {
    }

    public HighlightField(String name, Text[] fragments) {
        this.name = Objects.requireNonNull(name, "missing highlight field name");
        this.fragments = fragments;
    }

    
    public String name() {
        return name;
    }

    
    public String getName() {
        return name();
    }

    
    public Text[] fragments() {
        return fragments;
    }

    
    public Text[] getFragments() {
        return fragments();
    }

    @Override
    public String toString() {
        return "[" + name + "], fragments[" + Arrays.toString(fragments) + "]";
    }

    public static HighlightField readHighlightField(StreamInput in) throws IOException {
        HighlightField field = new HighlightField();
        field.readFrom(in);
        return field;
    }

    @Override
    public void readFrom(StreamInput in) throws IOException {
        name = in.readString();
        if (in.readBoolean()) {
            int size = in.readVInt();
            if (size == 0) {
                fragments = Text.EMPTY_ARRAY;
            } else {
                fragments = new Text[size];
                for (int i = 0; i < size; i++) {
                    fragments[i] = in.readText();
                }
            }
        }
    }

    @Override
    public void writeTo(StreamOutput out) throws IOException {
        out.writeString(name);
        if (fragments == null) {
            out.writeBoolean(false);
        } else {
            out.writeBoolean(true);
            out.writeVInt(fragments.length);
            for (Text fragment : fragments) {
                out.writeText(fragment);
            }
        }
    }

    public static HighlightField fromXContent(XContentParser parser) throws IOException {
        ensureExpectedToken(XContentParser.Token.FIELD_NAME, parser.currentToken(), parser::getTokenLocation);
        String fieldName = parser.currentName();
        Text[] fragments = null;
        XContentParser.Token token = parser.nextToken();
        if (token == XContentParser.Token.START_ARRAY) {
            List<Text> values = new ArrayList<>();
            while (parser.nextToken() != XContentParser.Token.END_ARRAY) {
                values.add(new Text(parser.text()));
            }
            fragments = values.toArray(new Text[values.size()]);
        } else if (token == XContentParser.Token.VALUE_NULL) {
            fragments = null;
        } else {
            throw new ParsingException(parser.getTokenLocation(),
                    "unexpected token type [" + token + "]");
        }
        return new HighlightField(fieldName, fragments);
    }

    @Override
    public XContentBuilder toXContent(XContentBuilder builder, Params params) throws IOException {
        builder.field(name);
        if (fragments == null) {
            builder.nullValue();
        } else {
            builder.startArray();
            for (Text fragment : fragments) {
                builder.value(fragment);
            }
            builder.endArray();
        }
        return builder;
    }

    @Override
    public final boolean equals(Object obj) {
        if (this == obj) {
            return true;
        }
        if (obj == null || getClass() != obj.getClass()) {
            return false;
        }
        HighlightField other = (HighlightField) obj;
        return Objects.equals(name, other.name) && Arrays.equals(fragments, other.fragments);
    }

    @Override
    public final int hashCode() {
        return Objects.hash(name, Arrays.hashCode(fragments));
    }

}
