

package org.elasticsearch.client.sniff;

import org.apache.http.HttpHost;

import java.io.IOException;
import java.util.List;


public interface HostsSniffer {
    
    List<HttpHost> sniffHosts() throws IOException;
}
