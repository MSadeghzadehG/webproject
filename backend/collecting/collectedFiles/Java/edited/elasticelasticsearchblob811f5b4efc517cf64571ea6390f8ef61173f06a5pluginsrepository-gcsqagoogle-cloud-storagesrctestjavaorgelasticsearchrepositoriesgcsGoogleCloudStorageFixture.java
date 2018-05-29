
package org.elasticsearch.repositories.gcs;

import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;
import org.elasticsearch.common.SuppressForbidden;
import org.elasticsearch.core.internal.io.Streams;
import org.elasticsearch.mocksocket.MockHttpServer;
import org.elasticsearch.repositories.gcs.GoogleCloudStorageTestServer.Response;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.lang.management.ManagementFactory;
import java.net.Inet6Address;
import java.net.InetAddress;
import java.net.InetSocketAddress;
import java.net.SocketAddress;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.nio.file.StandardCopyOption;
import java.util.List;
import java.util.Map;

import static java.util.Collections.singleton;
import static java.util.Collections.singletonList;


public class GoogleCloudStorageFixture {

    public static void main(String[] args) throws Exception {
        if (args == null || args.length != 2) {
            throw new IllegalArgumentException("GoogleCloudStorageFixture <working directory> <bucket>");
        }

        final InetSocketAddress socketAddress = new InetSocketAddress(InetAddress.getLoopbackAddress(), 0);
        final HttpServer httpServer = MockHttpServer.createHttp(socketAddress, 0);

        try {
            final Path workingDirectory = workingDir(args[0]);
                        writeFile(workingDirectory, "pid", ManagementFactory.getRuntimeMXBean().getName().split("@")[0]);

            final String addressAndPort = addressToString(httpServer.getAddress());
                        writeFile(workingDirectory, "ports", addressAndPort);

                        final String storageUrl = "http:            final GoogleCloudStorageTestServer storageTestServer = new GoogleCloudStorageTestServer(storageUrl);
            storageTestServer.createBucket(args[1]);

            httpServer.createContext("/", new ResponseHandler(storageTestServer));
            httpServer.start();

                        Thread.sleep(Long.MAX_VALUE);

        } finally {
            httpServer.stop(0);
        }
    }

    @SuppressForbidden(reason = "Paths#get is fine - we don't have environment here")
    private static Path workingDir(final String dir) {
        return Paths.get(dir);
    }

    private static void writeFile(final Path dir, final String fileName, final String content) throws IOException {
        final Path tempPidFile = Files.createTempFile(dir, null, null);
        Files.write(tempPidFile, singleton(content));
        Files.move(tempPidFile, dir.resolve(fileName), StandardCopyOption.ATOMIC_MOVE);
    }

    private static String addressToString(final SocketAddress address) {
        final InetSocketAddress inetSocketAddress = (InetSocketAddress) address;
        if (inetSocketAddress.getAddress() instanceof Inet6Address) {
            return "[" + inetSocketAddress.getHostString() + "]:" + inetSocketAddress.getPort();
        } else {
            return inetSocketAddress.getHostString() + ":" + inetSocketAddress.getPort();
        }
    }

    static class ResponseHandler implements HttpHandler {

        private final GoogleCloudStorageTestServer storageServer;

        private ResponseHandler(final GoogleCloudStorageTestServer storageServer) {
            this.storageServer = storageServer;
        }

        @Override
        public void handle(HttpExchange exchange) throws IOException {
            String method = exchange.getRequestMethod();
            String path = storageServer.getEndpoint() + exchange.getRequestURI().getRawPath();
            String query = exchange.getRequestURI().getRawQuery();
            Map<String, List<String>> headers = exchange.getRequestHeaders();

            ByteArrayOutputStream out = new ByteArrayOutputStream();
            Streams.copy(exchange.getRequestBody(), out);

            final Response storageResponse = storageServer.handle(method, path, query, headers, out.toByteArray());

            Map<String, List<String>> responseHeaders = exchange.getResponseHeaders();
            responseHeaders.put("Content-Type", singletonList(storageResponse.contentType));
            storageResponse.headers.forEach((k, v) -> responseHeaders.put(k, singletonList(v)));
            exchange.sendResponseHeaders(storageResponse.status.getStatus(), storageResponse.body.length);
            if (storageResponse.body.length > 0) {
                exchange.getResponseBody().write(storageResponse.body);
            }
            exchange.close();
        }
    }
}
