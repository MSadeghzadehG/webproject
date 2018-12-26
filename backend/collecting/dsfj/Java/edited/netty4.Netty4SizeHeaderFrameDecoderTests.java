

package org.elasticsearch.transport.netty4;

import org.elasticsearch.common.io.stream.NamedWriteableRegistry;
import org.elasticsearch.common.network.NetworkService;
import org.elasticsearch.common.settings.Settings;
import org.elasticsearch.common.transport.TransportAddress;
import org.elasticsearch.common.util.BigArrays;
import org.elasticsearch.common.util.MockBigArrays;
import org.elasticsearch.common.util.MockPageCacheRecycler;
import org.elasticsearch.indices.breaker.NoneCircuitBreakerService;
import org.elasticsearch.mocksocket.MockSocket;
import org.elasticsearch.test.ESTestCase;
import org.elasticsearch.threadpool.ThreadPool;
import org.elasticsearch.transport.TcpTransport;
import org.junit.After;
import org.junit.Before;

import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.net.InetAddress;
import java.net.Socket;
import java.nio.charset.StandardCharsets;
import java.util.Collections;

import static org.hamcrest.Matchers.is;


public class Netty4SizeHeaderFrameDecoderTests extends ESTestCase {

    private final Settings settings = Settings.builder()
        .put("node.name", "NettySizeHeaderFrameDecoderTests")
        .put(TcpTransport.BIND_HOST.getKey(), "127.0.0.1")
        .put(TcpTransport.PORT.getKey(), "0")
        .build();

    private ThreadPool threadPool;
    private Netty4Transport nettyTransport;
    private int port;
    private InetAddress host;

    @Before
    public void startThreadPool() {
        threadPool = new ThreadPool(settings);
        NetworkService networkService = new NetworkService(Collections.emptyList());
        BigArrays bigArrays = new MockBigArrays(new MockPageCacheRecycler(Settings.EMPTY), new NoneCircuitBreakerService());
        nettyTransport = new Netty4Transport(settings, threadPool, networkService, bigArrays,
            new NamedWriteableRegistry(Collections.emptyList()), new NoneCircuitBreakerService());
        nettyTransport.start();

        TransportAddress[] boundAddresses = nettyTransport.boundAddress().boundAddresses();
        TransportAddress transportAddress = (TransportAddress) randomFrom(boundAddresses);
        port = transportAddress.address().getPort();
        host = transportAddress.address().getAddress();
    }

    @After
    public void terminateThreadPool() throws InterruptedException {
        nettyTransport.stop();
        terminate(threadPool);
        threadPool = null;
    }

    public void testThatTextMessageIsReturnedOnHTTPLikeRequest() throws Exception {
        String randomMethod = randomFrom("GET", "POST", "PUT", "DELETE", "HEAD", "OPTIONS", "PATCH");
        String data = randomMethod + " / HTTP/1.1";

        try (Socket socket = new MockSocket(host, port)) {
            socket.getOutputStream().write(data.getBytes(StandardCharsets.UTF_8));
            socket.getOutputStream().flush();

            try (BufferedReader reader = new BufferedReader(new InputStreamReader(socket.getInputStream(), StandardCharsets.UTF_8))) {
                assertThat(reader.readLine(), is("This is not a HTTP port"));
            }
        }
    }

    public void testThatNothingIsReturnedForOtherInvalidPackets() throws Exception {
        try (Socket socket = new MockSocket(host, port)) {
            socket.getOutputStream().write("FOOBAR".getBytes(StandardCharsets.UTF_8));
            socket.getOutputStream().flush();

                        assertThat(socket.getInputStream().read(), is(-1));
        }
    }

}
