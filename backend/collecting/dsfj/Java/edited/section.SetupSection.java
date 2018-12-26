
package org.elasticsearch.test.rest.yaml.section;

import org.elasticsearch.common.xcontent.XContentParser;

import java.io.IOException;
import java.util.ArrayList;
import java.util.List;


public class SetupSection {
    
    public static SetupSection parseIfNext(XContentParser parser) throws IOException {
        ParserUtils.advanceToFieldName(parser);

        if ("setup".equals(parser.currentName())) {
            parser.nextToken();
            SetupSection section = parse(parser);
            parser.nextToken();
            return section;
        }

        return EMPTY;
    }

    public static SetupSection parse(XContentParser parser) throws IOException {
        SetupSection setupSection = new SetupSection();
        setupSection.setSkipSection(SkipSection.parseIfNext(parser));

        while (parser.currentToken() != XContentParser.Token.END_ARRAY) {
            ParserUtils.advanceToFieldName(parser);
            if (!"do".equals(parser.currentName())) {
                throw new IllegalArgumentException("section [" + parser.currentName() + "] not supported within setup section");
            }

            setupSection.addDoSection(DoSection.parse(parser));
            parser.nextToken();
        }

        parser.nextToken();

        return setupSection;
    }

    public static final SetupSection EMPTY;

    static {
        EMPTY = new SetupSection();
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
