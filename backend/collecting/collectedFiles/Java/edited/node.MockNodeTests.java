

package org.elasticsearch.node;

import org.elasticsearch.common.settings.Settings;
import org.elasticsearch.common.util.BigArrays;
import org.elasticsearch.common.util.MockBigArrays;
import org.elasticsearch.env.Environment;
import org.elasticsearch.plugins.Plugin;
import org.elasticsearch.search.MockSearchService;
import org.elasticsearch.search.SearchService;
import org.elasticsearch.test.ESTestCase;

import java.io.IOException;
import java.util.ArrayList;
import java.util.List;

public class MockNodeTests extends ESTestCase {
    
    public void testComponentsMockedByMarkerPlugins() throws IOException {
        Settings settings = Settings.builder()                 .put(Environment.PATH_HOME_SETTING.getKey(), createTempDir())
                .put("transport.type", getTestTransportType())
                .put("http.enabled", false)
                .build();
        List<Class<? extends Plugin>> plugins = new ArrayList<>();
        plugins.add(getTestTransportPlugin());
        boolean useMockBigArrays = randomBoolean();
        boolean useMockSearchService = randomBoolean();
        if (useMockBigArrays) {
            plugins.add(NodeMocksPlugin.class);
        }
        if (useMockSearchService) {
            plugins.add(MockSearchService.TestPlugin.class);
        }
        try (MockNode node = new MockNode(settings, plugins)) {
            BigArrays bigArrays = node.injector().getInstance(BigArrays.class);
            SearchService searchService = node.injector().getInstance(SearchService.class);
            if (useMockBigArrays) {
                assertSame(bigArrays.getClass(), MockBigArrays.class);
            } else {
                assertSame(bigArrays.getClass(), BigArrays.class);
            }
            if (useMockSearchService) {
                assertSame(searchService.getClass(), MockSearchService.class);
            } else {
                assertSame(searchService.getClass(), SearchService.class);
            }
        }
    }
}
