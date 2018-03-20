

package org.elasticsearch.indices.mapping;


import org.elasticsearch.action.ActionListener;
import org.elasticsearch.action.index.IndexResponse;
import org.elasticsearch.common.xcontent.XContentType;
import org.elasticsearch.index.query.QueryBuilders;
import org.elasticsearch.test.ESIntegTestCase;

import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.concurrent.CopyOnWriteArrayList;
import java.util.concurrent.CountDownLatch;

import static org.elasticsearch.test.hamcrest.ElasticsearchAssertions.assertAcked;
import static org.elasticsearch.test.hamcrest.ElasticsearchAssertions.assertHitCount;
import static org.hamcrest.Matchers.emptyIterable;

public class ConcurrentDynamicTemplateIT extends ESIntegTestCase {
    private final String mappingType = "test-mapping";

        public void testConcurrentDynamicMapping() throws Exception {
        final String fieldName = "field";
        final String mapping = "{ \"" + mappingType + "\": {" +
                "\"dynamic_templates\": ["
                + "{ \"" + fieldName + "\": {" + "\"path_match\": \"*\"," + "\"mapping\": {" + "\"type\": \"text\"," + "\"store\": true,"
                + "\"analyzer\": \"whitespace\" } } } ] } }";
                
        int iters = scaledRandomIntBetween(5, 15);
        for (int i = 0; i < iters; i++) {
            cluster().wipeIndices("test");
            assertAcked(prepareCreate("test")
                    .addMapping(mappingType, mapping, XContentType.JSON));
            int numDocs = scaledRandomIntBetween(10, 100);
            final CountDownLatch latch = new CountDownLatch(numDocs);
            final List<Throwable> throwable = new CopyOnWriteArrayList<>();
            int currentID = 0;
            for (int j = 0; j < numDocs; j++) {
                Map<String, Object> source = new HashMap<>();
                source.put(fieldName, "test-user");
                client().prepareIndex("test", mappingType, Integer.toString(currentID++)).setSource(source).execute(new ActionListener<IndexResponse>() {
                    @Override
                    public void onResponse(IndexResponse response) {
                        latch.countDown();
                    }

                    @Override
                    public void onFailure(Exception e) {
                        throwable.add(e);
                        latch.countDown();
                    }
                });
            }
            latch.await();
            assertThat(throwable, emptyIterable());
            refresh();
            assertHitCount(client().prepareSearch("test").setQuery(QueryBuilders.matchQuery(fieldName, "test-user")).get(), numDocs);
            assertHitCount(client().prepareSearch("test").setQuery(QueryBuilders.matchQuery(fieldName, "test user")).get(), 0);

        }
    }
}
