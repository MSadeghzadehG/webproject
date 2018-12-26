

package org.elasticsearch.test.rest.yaml.section;

import org.elasticsearch.common.ParsingException;
import org.elasticsearch.common.xcontent.XContentParser;

import java.io.IOException;
import java.util.ArrayList;
import java.util.List;

public class TeardownSection {
    
    public static TeardownSection parseIfNext(XContentParser parser) throws IOException {
        ParserUtils.advanceToFieldName(parser);

        if ("teardown".equals(parser.currentName())) {
            parser.nextToken();
            TeardownSection section = parse(parser);
            parser.nextToken();
            return section;
        }

        return EMPTY;
    }

    public static TeardownSection parse(XContentParser parser) throws IOException {
        TeardownSection teardownSection = new TeardownSection();
        teardownSection.setSkipSection(SkipSection.parseIfNext(parser));

        while (parser.currentToken() != XContentParser.Token.END_ARRAY) {
            ParserUtils.advanceToFieldName(parser);
            if (!"do".equals(parser.currentName())) {
                throw new ParsingException(parser.getTokenLocation(),
                        "section [" + parser.currentName() + "] not supported within teardown section");
            }

            teardownSection.addDoSection(DoSection.parse(parser));
            parser.nextToken();
        }

        parser.nextToken();
        return teardownSection;
    }

    public static final TeardownSection EMPTY;

    static {
        EMPTY = new TeardownSection();
        EMPTY.setSkipSection(SkipSection.EMPTY);
    }

    private SkipSection skipSection;
    private List<DoSection> doSections = new ArrayList<>();

    public SkipSection getSkipSection() {
        return skipSection;
    }

    public void setSkipSection(SkipSection skipSection) {
        this.skipSection = skipSection;
    }

    public List<DoSection> getDoSections() {
        return doSections;
    }

    public void addDoSection(DoSection doSection) {
        this.doSections.add(doSection);
    }

    public boolean isEmpty() {
        return EMPTY.equals(this);
    }
}
