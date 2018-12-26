

package org.elasticsearch.search.query;

import org.elasticsearch.action.explain.ExplainResponse;
import org.elasticsearch.action.index.IndexRequestBuilder;
import org.elasticsearch.action.search.SearchResponse;
import org.elasticsearch.common.Strings;
import org.elasticsearch.common.xcontent.XContentBuilder;
import org.elasticsearch.common.xcontent.json.JsonXContent;
import org.elasticsearch.index.query.QueryBuilders;
import org.elasticsearch.search.SearchHit;
import org.elasticsearch.test.ESIntegTestCase;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Locale;
import java.util.Map;

import static java.util.Collections.emptyMap;
import static java.util.Collections.singletonMap;
import static org.elasticsearch.test.hamcrest.ElasticsearchAssertions.assertAcked;
import static org.elasticsearch.test.hamcrest.ElasticsearchAssertions.assertHitCount;
import static org.elasticsearch.test.hamcrest.ElasticsearchAssertions.assertSearchResponse;

public class ExistsIT extends ESIntegTestCase {

        public void testEmptyIndex() throws Exception {
        createIndex("test");
        SearchResponse resp = client().prepareSearch("test").setQuery(QueryBuilders.existsQuery("foo")).execute().actionGet();
        assertSearchResponse(resp);
        resp = client().prepareSearch("test").setQuery(QueryBuilders.boolQuery().mustNot(QueryBuilders.existsQuery("foo"))).execute().actionGet();
        assertSearchResponse(resp);
    }

    public void testExists() throws Exception {
        XContentBuilder mapping = XContentBuilder.builder(JsonXContent.jsonXContent)
            .startObject()
                .startObject("type")
                    .startObject("properties")
                        .startObject("foo")
                            .field("type", "text")
                        .endObject()
                        .startObject("bar")
                            .field("type", "object")
                            .startObject("properties")
                                .startObject("foo")
                                    .field("type", "text")
                                .endObject()
                                .startObject("bar")
                                    .field("type", "object")
                                    .startObject("properties")
                                        .startObject("bar")
                                            .field("type", "text")
                                        .endObject()
                                    .endObject()
                                .endObject()
                                .startObject("baz")
                                    .field("type", "long")
                                .endObject()
                            .endObject()
                        .endObject()
                    .endObject()
                .endObject()
            .endObject();

        assertAcked(client().admin().indices().prepareCreate("idx").addMapping("type", mapping));
        Map<String, Object> barObject = new HashMap<>();
        barObject.put("foo", "bar");
        barObject.put("bar", singletonMap("bar", "foo"));
        @SuppressWarnings("unchecked")
        final Map<String, Object>[] sources = new Map[] {
                                singletonMap("foo", "bar"),
                                singletonMap("bar", barObject),
                singletonMap("bar", singletonMap("baz", 42)),
                                emptyMap()
        };
        List<IndexRequestBuilder> reqs = new ArrayList<>();
        for (Map<String, Object> source : sources) {
            reqs.add(client().prepareIndex("idx", "type").setSource(source));
        }
                                indexRandom(true, false, reqs);

        final Map<String, Integer> expected = new LinkedHashMap<>();
        expected.put("foo", 1);
        expected.put("f*", 1);
        expected.put("bar", 2);
        expected.put("bar.*", 2);
        expected.put("bar.foo", 1);
        expected.put("bar.bar", 1);
        expected.put("bar.bar.bar", 1);
        expected.put("foobar", 0);

        final long numDocs = sources.length;
        SearchResponse allDocs = client().prepareSearch("idx").setSize(sources.length).get();
        assertSearchResponse(allDocs);
        assertHitCount(allDocs, numDocs);
        for (Map.Entry<String, Integer> entry : expected.entrySet()) {
            final String fieldName = entry.getKey();
            final int count = entry.getValue();
                        SearchResponse resp = client().prepareSearch("idx").setQuery(QueryBuilders.existsQuery(fieldName)).execute().actionGet();
            assertSearchResponse(resp);
            try {
                assertEquals(String.format(Locale.ROOT, "exists(%s, %d) mapping: %s response: %s", fieldName, count, Strings.toString(mapping), resp), count, resp.getHits().getTotalHits());
            } catch (AssertionError e) {
                for (SearchHit searchHit : allDocs.getHits()) {
                    final String index = searchHit.getIndex();
                    final String type = searchHit.getType();
                    final String id = searchHit.getId();
                    final ExplainResponse explanation = client().prepareExplain(index, type, id).setQuery(QueryBuilders.existsQuery(fieldName)).get();
                    logger.info("Explanation for [{}] / [{}] / [{}]: [{}]", fieldName, id, searchHit.getSourceAsString(), explanation.getExplanation());
                }
                throw e;
            }
        }
    }
}
