

package org.elasticsearch.index.store;

import org.elasticsearch.action.search.SearchResponse;
import org.elasticsearch.plugin.store.smb.SMBStorePlugin;
import org.elasticsearch.plugins.Plugin;
import org.elasticsearch.test.ESIntegTestCase;

import java.util.Arrays;
import java.util.Collection;

import static org.hamcrest.Matchers.is;

public abstract  class AbstractAzureFsTestCase extends ESIntegTestCase {
    @Override
    protected Collection<Class<? extends Plugin>> nodePlugins() {
        return Arrays.asList(SMBStorePlugin.class);
    }

    public void testAzureFs() {
                createIndex("test");
        long nbDocs = randomIntBetween(10, 1000);
        for (long i = 0; i < nbDocs; i++) {
            index("test", "doc", "" + i, "foo", "bar");
        }
        refresh();
        SearchResponse response = client().prepareSearch("test").get();
        assertThat(response.getHits().getTotalHits(), is(nbDocs));
    }
}
