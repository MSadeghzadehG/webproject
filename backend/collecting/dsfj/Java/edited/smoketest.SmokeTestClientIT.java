

package org.elasticsearch.smoketest;

import org.elasticsearch.action.admin.cluster.health.ClusterHealthResponse;
import org.elasticsearch.action.search.SearchResponse;
import org.elasticsearch.client.Client;

import static org.hamcrest.CoreMatchers.is;
import static org.hamcrest.Matchers.greaterThan;

public class SmokeTestClientIT extends ESSmokeClientTestCase {

    
    public void testSimpleClient() {
        final Client client = getClient();

                final ClusterHealthResponse health =
                client.admin().cluster().prepareHealth().setWaitForYellowStatus().get();
        final String clusterName = health.getClusterName();
        final int numberOfNodes = health.getNumberOfNodes();
                assertThat(
                "cluster [" + clusterName + "] should have at least 1 node",
                numberOfNodes,
                greaterThan(0));
    }

    
    public void testPutDocument() {
        final Client client = getClient();

                client.prepareIndex(index, "doc", "1")                  .setSource("foo", "bar")                        .get();                                 
                        client.admin().indices().prepareRefresh(index).get();
        
                final SearchResponse searchResponse = client.prepareSearch(index).get();
        assertThat(searchResponse.getHits().getTotalHits(), is(1L));
            }

}

