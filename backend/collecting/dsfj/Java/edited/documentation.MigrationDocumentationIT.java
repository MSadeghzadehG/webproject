

package org.elasticsearch.client.documentation;

import org.apache.http.HttpEntity;
import org.apache.http.HttpStatus;
import org.apache.http.entity.ContentType;
import org.apache.http.nio.entity.NStringEntity;
import org.elasticsearch.action.ActionListener;
import org.elasticsearch.action.delete.DeleteRequest;
import org.elasticsearch.action.delete.DeleteResponse;
import org.elasticsearch.action.get.GetRequest;
import org.elasticsearch.action.index.IndexRequest;
import org.elasticsearch.action.index.IndexResponse;
import org.elasticsearch.client.ESRestHighLevelClientTestCase;
import org.elasticsearch.client.Response;
import org.elasticsearch.client.RestHighLevelClient;
import org.elasticsearch.cluster.health.ClusterHealthStatus;
import org.elasticsearch.common.Strings;
import org.elasticsearch.common.settings.Settings;
import org.elasticsearch.common.xcontent.XContentFactory;
import org.elasticsearch.common.xcontent.XContentHelper;
import org.elasticsearch.common.xcontent.XContentType;
import org.elasticsearch.rest.RestStatus;

import java.io.IOException;
import java.io.InputStream;
import java.util.Map;

import static java.util.Collections.emptyMap;
import static java.util.Collections.singletonMap;
import static org.elasticsearch.cluster.metadata.IndexMetaData.SETTING_NUMBER_OF_REPLICAS;
import static org.elasticsearch.cluster.metadata.IndexMetaData.SETTING_NUMBER_OF_SHARDS;


public class MigrationDocumentationIT extends ESRestHighLevelClientTestCase {

    public void testCreateIndex() throws IOException {
        RestHighLevelClient client = highLevelClient();
        {
                        Settings indexSettings = Settings.builder()                     .put(SETTING_NUMBER_OF_SHARDS, 1)
                    .put(SETTING_NUMBER_OF_REPLICAS, 0)
                    .build();

            String payload = Strings.toString(XContentFactory.jsonBuilder()                     .startObject()
                        .startObject("settings")                             .value(indexSettings)
                        .endObject()
                        .startObject("mappings")                              .startObject("doc")
                                .startObject("properties")
                                    .startObject("time")
                                        .field("type", "date")
                                    .endObject()
                                .endObject()
                            .endObject()
                        .endObject()
                    .endObject());

            HttpEntity entity = new NStringEntity(payload, ContentType.APPLICATION_JSON); 
            Response response = client.getLowLevelClient().performRequest("PUT", "my-index", emptyMap(), entity);             if (response.getStatusLine().getStatusCode() != HttpStatus.SC_OK) {
                            }
                        assertEquals(200, response.getStatusLine().getStatusCode());
        }
    }

    public void testClusterHealth() throws IOException {
        RestHighLevelClient client = highLevelClient();
        {
                        Map<String, String> parameters = singletonMap("wait_for_status", "green");
            Response response = client.getLowLevelClient().performRequest("GET", "/_cluster/health", parameters); 
            ClusterHealthStatus healthStatus;
            try (InputStream is = response.getEntity().getContent()) {                 Map<String, Object> map = XContentHelper.convertToMap(XContentType.JSON.xContent(), is, true);                 healthStatus = ClusterHealthStatus.fromString((String) map.get("status"));             }

            if (healthStatus == ClusterHealthStatus.GREEN) {
                            }
                        assertSame(ClusterHealthStatus.GREEN, healthStatus);
        }
    }

    public void testRequests() throws Exception {
        RestHighLevelClient client = highLevelClient();
        {
                        IndexRequest request = new IndexRequest("index", "doc", "id");             request.source("{\"field\":\"value\"}", XContentType.JSON);
            
                        IndexResponse response = client.index(request);
                        assertEquals(RestStatus.CREATED, response.status());
        }
        {
                        DeleteRequest request = new DeleteRequest("index", "doc", "id");             client.deleteAsync(request, new ActionListener<DeleteResponse>() {                 @Override
                public void onResponse(DeleteResponse deleteResponse) {
                                    }

                @Override
                public void onFailure(Exception e) {
                                    }
            });
                        assertBusy(() -> assertFalse(client.exists(new GetRequest("index", "doc", "id"))));
        }
        {
                        DeleteRequest request = new DeleteRequest("index", "doc", "id");
            DeleteResponse response = client.delete(request);                         assertEquals(RestStatus.NOT_FOUND, response.status());
        }
    }
}
