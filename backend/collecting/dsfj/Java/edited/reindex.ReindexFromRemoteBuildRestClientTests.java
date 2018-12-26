

package org.elasticsearch.index.reindex;

import org.elasticsearch.client.RestClient;
import org.elasticsearch.client.RestClientBuilderTestCase;
import org.elasticsearch.common.bytes.BytesArray;
import org.elasticsearch.test.ESTestCase;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import static java.util.Collections.emptyMap;
import static java.util.Collections.synchronizedList;
import static org.hamcrest.Matchers.hasSize;

public class ReindexFromRemoteBuildRestClientTests extends RestClientBuilderTestCase {
    public void testBuildRestClient() throws Exception {
        RemoteInfo remoteInfo = new RemoteInfo("https", "localhost", 9200, new BytesArray("ignored"), null, null, emptyMap(),
                RemoteInfo.DEFAULT_SOCKET_TIMEOUT, RemoteInfo.DEFAULT_CONNECT_TIMEOUT);
        long taskId = randomLong();
        List<Thread> threads = synchronizedList(new ArrayList<>());
        RestClient client = TransportReindexAction.buildRestClient(remoteInfo, taskId, threads);
        try {
            assertBusy(() -> assertThat(threads, hasSize(2)));
            int i = 0;
            for (Thread thread : threads) {
                assertEquals("es-client-" + taskId + "-" + i, thread.getName());
                i++;
            }
        } finally {
            client.close();
        }
    }

    public void testHeaders() throws Exception {
        Map<String, String> headers = new HashMap<>();
        int numHeaders = randomIntBetween(1, 5);
        for (int i = 0; i < numHeaders; ++i) {
            headers.put("header" + i, Integer.toString(i));
        }
        RemoteInfo remoteInfo = new RemoteInfo("https", "localhost", 9200, new BytesArray("ignored"), null, null,
            headers, RemoteInfo.DEFAULT_SOCKET_TIMEOUT, RemoteInfo.DEFAULT_CONNECT_TIMEOUT);
        long taskId = randomLong();
        List<Thread> threads = synchronizedList(new ArrayList<>());
        RestClient client = TransportReindexAction.buildRestClient(remoteInfo, taskId, threads);
        try {
            assertHeaders(client, headers);
        } finally {
            client.close();
        }
    }
}
