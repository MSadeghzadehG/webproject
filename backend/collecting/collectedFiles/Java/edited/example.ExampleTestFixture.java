

package example;

import java.lang.management.ManagementFactory;
import java.net.Inet6Address;
import java.net.InetAddress;
import java.net.InetSocketAddress;
import java.nio.ByteBuffer;
import java.nio.channels.AsynchronousServerSocketChannel;
import java.nio.channels.AsynchronousSocketChannel;
import java.nio.channels.CompletionHandler;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.nio.file.StandardCopyOption;
import java.util.Collections;


public class ExampleTestFixture {
    public static void main(String args[]) throws Exception {
        if (args.length != 1) {
            throw new IllegalArgumentException("ExampleTestFixture <logDirectory>");
        }
        Path dir = Paths.get(args[0]);
        AsynchronousServerSocketChannel server = AsynchronousServerSocketChannel
                .open()
                .bind(new InetSocketAddress(InetAddress.getLoopbackAddress(), 0));

                Path tmp = Files.createTempFile(dir, null, null);
        String pid = ManagementFactory.getRuntimeMXBean().getName().split("@")[0];
        Files.write(tmp, Collections.singleton(pid));
        Files.move(tmp, dir.resolve("pid"), StandardCopyOption.ATOMIC_MOVE);

                tmp = Files.createTempFile(dir, null, null);
        InetSocketAddress bound = (InetSocketAddress) server.getLocalAddress();
        if (bound.getAddress() instanceof Inet6Address) {
            Files.write(tmp, Collections.singleton("[" + bound.getHostString() + "]:" + bound.getPort()));
        } else {
            Files.write(tmp, Collections.singleton(bound.getHostString() + ":" + bound.getPort()));
        }
        Files.move(tmp, dir.resolve("ports"), StandardCopyOption.ATOMIC_MOVE);

                server.accept(null, new CompletionHandler<AsynchronousSocketChannel,Void>() {
            @Override
            public void completed(AsynchronousSocketChannel socket, Void attachment) {
                server.accept(null, this);
                try (AsynchronousSocketChannel ch = socket) {
                    ch.write(ByteBuffer.wrap("TEST\n".getBytes(StandardCharsets.UTF_8))).get();
                } catch (Exception e) {
                    throw new RuntimeException(e);
                }
            }

            @Override
            public void failed(Throwable exc, Void attachment) {}
        });

                Thread.sleep(Long.MAX_VALUE);
    }
}
