

package org.elasticsearch.index.reindex;

import org.elasticsearch.common.settings.Settings;
import org.elasticsearch.common.xcontent.NamedXContentRegistry;
import org.elasticsearch.rest.RestController;
import org.elasticsearch.test.ESTestCase;
import org.elasticsearch.test.rest.FakeRestRequest;

import java.io.IOException;

import static java.util.Collections.emptyList;
import static org.mockito.Mockito.mock;

public class RestUpdateByQueryActionTests extends ESTestCase {
    public void testParseEmpty() throws IOException {
        RestUpdateByQueryAction action = new RestUpdateByQueryAction(Settings.EMPTY, mock(RestController.class));
        UpdateByQueryRequest request = action.buildRequest(new FakeRestRequest.Builder(new NamedXContentRegistry(emptyList()))
                .build());
        assertEquals(AbstractBulkByScrollRequest.SIZE_ALL_MATCHES, request.getSize());
        assertEquals(AbstractBulkByScrollRequest.DEFAULT_SCROLL_SIZE, request.getSearchRequest().source().size());
    }
}
