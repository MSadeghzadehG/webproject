

package org.elasticsearch.client.sniff;

import org.apache.http.HttpHost;

import java.io.IOException;
import java.util.Collections;
import java.util.List;


class MockHostsSniffer implements HostsSniffer {
    @Override
    public List<HttpHost> sniffHosts() throws IOException {
        return Collections.singletonList(new HttpHost("localhost", 9200));
    }
}
