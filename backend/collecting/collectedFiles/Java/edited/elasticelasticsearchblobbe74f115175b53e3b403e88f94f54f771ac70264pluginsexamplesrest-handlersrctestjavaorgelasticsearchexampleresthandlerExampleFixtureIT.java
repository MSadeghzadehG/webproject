

package org.elasticsearch.example.resthandler;

import org.elasticsearch.mocksocket.MockSocket;
import org.elasticsearch.test.ESTestCase;

import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.net.InetAddress;
import java.net.Socket;
import java.net.URL;
import java.nio.charset.StandardCharsets;
import java.util.Objects;

public class ExampleFixtureIT extends ESTestCase {

    public void testExample() throws Exception {
        final String stringAddress = Objects.requireNonNull(System.getProperty("external.address"));
        final URL url = new URL("http:
        final InetAddress address = InetAddress.getByName(url.getHost());
        try (
            Socket socket = new MockSocket(address, url.getPort());
            BufferedReader reader = new BufferedReader(new InputStreamReader(socket.getInputStream(), StandardCharsets.UTF_8))
        ) {
            assertEquals("TEST", reader.readLine());
        }
    }
}
