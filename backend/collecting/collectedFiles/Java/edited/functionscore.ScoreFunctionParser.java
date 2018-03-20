

package org.elasticsearch.index.query.functionscore;

import org.elasticsearch.common.xcontent.XContentParser;

import java.io.IOException;


@FunctionalInterface
public interface ScoreFunctionParser<FB extends ScoreFunctionBuilder<FB>> {
    FB fromXContent(XContentParser parser) throws IOException;
}
