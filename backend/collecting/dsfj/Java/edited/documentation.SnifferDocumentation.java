

package org.elasticsearch.client.sniff.documentation;

import org.apache.http.HttpHost;
import org.elasticsearch.client.RestClient;
import org.elasticsearch.client.sniff.ElasticsearchHostsSniffer;
import org.elasticsearch.client.sniff.HostsSniffer;
import org.elasticsearch.client.sniff.SniffOnFailureListener;
import org.elasticsearch.client.sniff.Sniffer;

import java.io.IOException;
import java.util.List;
import java.util.concurrent.TimeUnit;


@SuppressWarnings("unused")
public class SnifferDocumentation {

    @SuppressWarnings("unused")
    public void testUsage() throws IOException {
        {
                        RestClient restClient = RestClient.builder(
                    new HttpHost("localhost", 9200, "http"))
                    .build();
            Sniffer sniffer = Sniffer.builder(restClient).build();
            
                        sniffer.close();
            restClient.close();
                    }
        {
                        RestClient restClient = RestClient.builder(
                    new HttpHost("localhost", 9200, "http"))
                    .build();
            Sniffer sniffer = Sniffer.builder(restClient)
                    .setSniffIntervalMillis(60000).build();
                    }
        {
                        SniffOnFailureListener sniffOnFailureListener = new SniffOnFailureListener();
            RestClient restClient = RestClient.builder(new HttpHost("localhost", 9200))
                    .setFailureListener(sniffOnFailureListener)                     .build();
            Sniffer sniffer = Sniffer.builder(restClient)
                    .setSniffAfterFailureDelayMillis(30000)                     .build();
            sniffOnFailureListener.setSniffer(sniffer);                     }
        {
                        RestClient restClient = RestClient.builder(
                    new HttpHost("localhost", 9200, "http"))
                    .build();
            HostsSniffer hostsSniffer = new ElasticsearchHostsSniffer(
                    restClient,
                    ElasticsearchHostsSniffer.DEFAULT_SNIFF_REQUEST_TIMEOUT,
                    ElasticsearchHostsSniffer.Scheme.HTTPS);
            Sniffer sniffer = Sniffer.builder(restClient)
                    .setHostsSniffer(hostsSniffer).build();
                    }
        {
                        RestClient restClient = RestClient.builder(
                    new HttpHost("localhost", 9200, "http"))
                    .build();
            HostsSniffer hostsSniffer = new ElasticsearchHostsSniffer(
                    restClient,
                    TimeUnit.SECONDS.toMillis(5),
                    ElasticsearchHostsSniffer.Scheme.HTTP);
            Sniffer sniffer = Sniffer.builder(restClient)
                    .setHostsSniffer(hostsSniffer).build();
                    }
        {
                        RestClient restClient = RestClient.builder(
                    new HttpHost("localhost", 9200, "http"))
                    .build();
            HostsSniffer hostsSniffer = new HostsSniffer() {
                @Override
                public List<HttpHost> sniffHosts() throws IOException {
                    return null;                 }
            };
            Sniffer sniffer = Sniffer.builder(restClient)
                    .setHostsSniffer(hostsSniffer).build();
                    }
    }
}
