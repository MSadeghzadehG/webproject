
package org.elasticsearch.transport;

import org.elasticsearch.core.internal.io.IOUtils;
import org.elasticsearch.Version;
import org.elasticsearch.action.ActionListener;
import org.elasticsearch.common.bytes.BytesReference;
import org.elasticsearch.common.io.stream.BytesStreamOutput;
import org.elasticsearch.common.io.stream.InputStreamStreamInput;
import org.elasticsearch.common.io.stream.NamedWriteableRegistry;
import org.elasticsearch.common.io.stream.StreamInput;
import org.elasticsearch.common.network.NetworkService;
import org.elasticsearch.common.settings.Settings;
import org.elasticsearch.common.unit.ByteSizeValue;
import org.elasticsearch.common.util.BigArrays;
import org.elasticsearch.common.util.CancellableThreads;
import org.elasticsearch.common.util.concurrent.AbstractRunnable;
import org.elasticsearch.common.util.concurrent.EsExecutors;
import org.elasticsearch.indices.breaker.CircuitBreakerService;
import org.elasticsearch.mocksocket.MockServerSocket;
import org.elasticsearch.mocksocket.MockSocket;
import org.elasticsearch.threadpool.ThreadPool;

import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.Closeable;
import java.io.IOException;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.net.ServerSocket;
import java.net.Socket;
import java.net.SocketException;
import java.util.Collections;
import java.util.HashSet;
import java.util.Set;
import java.util.concurrent.CompletableFuture;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.Executor;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.AtomicBoolean;


public class MockTcpTransport extends TcpTransport {

    
    public static final ConnectionProfile LIGHT_PROFILE;

    private final Set<MockChannel> openChannels = new HashSet<>();

    static {
        ConnectionProfile.Builder builder = new ConnectionProfile.Builder();
        builder.addConnections(1,
            TransportRequestOptions.Type.BULK,
            TransportRequestOptions.Type.PING,
            TransportRequestOptions.Type.RECOVERY,
            TransportRequestOptions.Type.REG,
            TransportRequestOptions.Type.STATE);
        LIGHT_PROFILE = builder.build();
    }

    private final ExecutorService executor;
    private final Version mockVersion;

    public MockTcpTransport(Settings settings, ThreadPool threadPool, BigArrays bigArrays,
                            CircuitBreakerService circuitBreakerService, NamedWriteableRegistry namedWriteableRegistry,
                            NetworkService networkService) {
        this(settings, threadPool, bigArrays, circuitBreakerService, namedWriteableRegistry, networkService,
            Version.CURRENT);
    }

    public MockTcpTransport(Settings settings, ThreadPool threadPool, BigArrays bigArrays,
                            CircuitBreakerService circuitBreakerService, NamedWriteableRegistry namedWriteableRegistry,
                            NetworkService networkService, Version mockVersion) {
        super("mock-tcp-transport", settings, threadPool, bigArrays, circuitBreakerService, namedWriteableRegistry, networkService);
                        executor = Executors.newCachedThreadPool(EsExecutors.daemonThreadFactory(settings, Transports.TEST_MOCK_TRANSPORT_THREAD_PREFIX));
        this.mockVersion = mockVersion;
    }

    @Override
    protected MockChannel bind(final String name, InetSocketAddress address) throws IOException {
        MockServerSocket socket = new MockServerSocket();
        socket.setReuseAddress(TCP_REUSE_ADDRESS.get(settings));
        ByteSizeValue tcpReceiveBufferSize = TCP_RECEIVE_BUFFER_SIZE.get(settings);
        if (tcpReceiveBufferSize.getBytes() > 0) {
            socket.setReceiveBufferSize(tcpReceiveBufferSize.bytesAsInt());
        }
        socket.bind(address);
        MockChannel serverMockChannel = new MockChannel(socket, name);
        CountDownLatch started = new CountDownLatch(1);
        executor.execute(new AbstractRunnable() {
            @Override
            public void onFailure(Exception e) {
                onException(serverMockChannel, e);
            }

            @Override
            protected void doRun() throws Exception {
                started.countDown();
                serverMockChannel.accept(executor);
            }
        });
        try {
            started.await();
        } catch (InterruptedException e) {
            Thread.currentThread().interrupt();
        }
        return serverMockChannel;
    }

    private void readMessage(MockChannel mockChannel, StreamInput input) throws IOException {
        Socket socket = mockChannel.activeChannel;
        byte[] minimalHeader = new byte[TcpHeader.MARKER_BYTES_SIZE];
        int firstByte = input.read();
        if (firstByte == -1) {
            throw new IOException("Connection reset by peer");
        }
        minimalHeader[0] = (byte) firstByte;
        minimalHeader[1] = (byte) input.read();
        int msgSize = input.readInt();
        if (msgSize == -1) {
            socket.getOutputStream().flush();
        } else {
            BytesStreamOutput output = new BytesStreamOutput();
            final byte[] buffer = new byte[msgSize];
            input.readFully(buffer);
            output.write(minimalHeader);
            output.writeInt(msgSize);
            output.write(buffer);
            consumeNetworkReads(mockChannel, output.bytes());
        }
    }

    @Override
    protected MockChannel initiateChannel(InetSocketAddress address, ActionListener<Void> connectListener) throws IOException {
        final MockSocket socket = new MockSocket();
        final MockChannel channel = new MockChannel(socket, address, "none");

        boolean success = false;
        try {
            configureSocket(socket);
            success = true;
        } finally {
            if (success == false) {
                IOUtils.close(socket);
            }

        }

        executor.submit(() -> {
            try {
                socket.connect(address);
                channel.loopRead(executor);
                connectListener.onResponse(null);
            } catch (Exception ex) {
                connectListener.onFailure(ex);
            }
        });

        return channel;
    }

    @Override
    protected ConnectionProfile resolveConnectionProfile(ConnectionProfile connectionProfile) {
        ConnectionProfile connectionProfile1 = resolveConnectionProfile(connectionProfile, defaultConnectionProfile);
        ConnectionProfile.Builder builder = new ConnectionProfile.Builder(LIGHT_PROFILE);
        builder.setHandshakeTimeout(connectionProfile1.getHandshakeTimeout());
        builder.setConnectTimeout(connectionProfile1.getConnectTimeout());
        return builder.build();
    }

    private void configureSocket(Socket socket) throws SocketException {
        socket.setTcpNoDelay(TCP_NO_DELAY.get(settings));
        ByteSizeValue tcpSendBufferSize = TCP_SEND_BUFFER_SIZE.get(settings);
        if (tcpSendBufferSize.getBytes() > 0) {
            socket.setSendBufferSize(tcpSendBufferSize.bytesAsInt());
        }
        ByteSizeValue tcpReceiveBufferSize = TCP_RECEIVE_BUFFER_SIZE.get(settings);
        if (tcpReceiveBufferSize.getBytes() > 0) {
            socket.setReceiveBufferSize(tcpReceiveBufferSize.bytesAsInt());
        }
        socket.setReuseAddress(TCP_REUSE_ADDRESS.get(settings));
    }

    public final class MockChannel implements Closeable, TcpChannel {
        private final AtomicBoolean isOpen = new AtomicBoolean(true);
        private final InetSocketAddress localAddress;
        private final ServerSocket serverSocket;
        private final Set<MockChannel> workerChannels = Collections.newSetFromMap(new ConcurrentHashMap<>());
        private final Socket activeChannel;
        private final String profile;
        private final CancellableThreads cancellableThreads = new CancellableThreads();
        private final CompletableFuture<Void> closeFuture = new CompletableFuture<>();

        
        public MockChannel(Socket socket, InetSocketAddress localAddress, String profile) {
            this.localAddress = localAddress;
            this.activeChannel = socket;
            this.serverSocket = null;
            this.profile = profile;
            synchronized (openChannels) {
                openChannels.add(this);
            }
        }

        
        MockChannel(ServerSocket serverSocket, String profile) {
            this.localAddress = (InetSocketAddress) serverSocket.getLocalSocketAddress();
            this.serverSocket = serverSocket;
            this.profile = profile;
            this.activeChannel = null;
            synchronized (openChannels) {
                openChannels.add(this);
            }
        }

        public void accept(Executor executor) throws IOException {
            while (isOpen.get()) {
                Socket incomingSocket = serverSocket.accept();
                MockChannel incomingChannel = null;
                try {
                    configureSocket(incomingSocket);
                    synchronized (this) {
                        if (isOpen.get()) {
                            incomingChannel = new MockChannel(incomingSocket,
                                new InetSocketAddress(incomingSocket.getLocalAddress(), incomingSocket.getPort()), profile);
                            MockChannel finalIncomingChannel = incomingChannel;
                            incomingChannel.addCloseListener(new ActionListener<Void>() {
                                @Override
                                public void onResponse(Void aVoid) {
                                    workerChannels.remove(finalIncomingChannel);
                                }

                                @Override
                                public void onFailure(Exception e) {
                                    workerChannels.remove(finalIncomingChannel);
                                }
                            });
                            serverAcceptedChannel(incomingChannel);
                                                        workerChannels.add(incomingChannel);

                                                        incomingChannel.loopRead(executor);
                                                        incomingSocket = null;
                            incomingChannel = null;
                        }
                    }
                } finally {
                                                            IOUtils.closeWhileHandlingException(incomingSocket, incomingChannel);
                }
            }
        }

        void loopRead(Executor executor) {
            executor.execute(new AbstractRunnable() {
                @Override
                public void onFailure(Exception e) {
                    if (isOpen.get()) {
                        try {
                            onException(MockChannel.this, e);
                        } catch (Exception ex) {
                            logger.warn("failed on handling exception", ex);
                            IOUtils.closeWhileHandlingException(MockChannel.this);                         }
                    }
                }

                @Override
                protected void doRun() throws Exception {
                    StreamInput input = new InputStreamStreamInput(new BufferedInputStream(activeChannel.getInputStream()));
                                        while (isOpen.get() && !Thread.currentThread().isInterrupted()) {
                        cancellableThreads.executeIO(() -> readMessage(MockChannel.this, input));
                    }
                }
            });
        }

        synchronized void close0() throws IOException {
                                                                                    if (isOpen.compareAndSet(true, false)) {
                final boolean removedChannel;
                synchronized (openChannels) {
                    removedChannel = openChannels.remove(this);
                }
                IOUtils.close(serverSocket, activeChannel, () -> IOUtils.close(workerChannels),
                    () -> cancellableThreads.cancel("channel closed"));
                assert removedChannel: "Channel was not removed or removed twice?";
            }
        }

        @Override
        public String toString() {
            return "MockChannel{" +
                "profile='" + profile + '\'' +
                ", isOpen=" + isOpen +
                ", localAddress=" + localAddress +
                ", isServerSocket=" + (serverSocket != null) +
                '}';
        }

        @Override
        public void close() {
            try {
                close0();
                closeFuture.complete(null);
            } catch (IOException e) {
                closeFuture.completeExceptionally(e);
            }
        }

        @Override
        public String getProfile() {
            return profile;
        }

        @Override
        public void addCloseListener(ActionListener<Void> listener) {
            closeFuture.whenComplete(ActionListener.toBiConsumer(listener));
        }

        @Override
        public void setSoLinger(int value) throws IOException {
            if (activeChannel != null && activeChannel.isClosed() == false) {
                activeChannel.setSoLinger(true, value);
            }

        }

        @Override
        public boolean isOpen() {
            return isOpen.get();
        }

        @Override
        public InetSocketAddress getLocalAddress() {
            return localAddress;
        }

        @Override
        public InetSocketAddress getRemoteAddress() {
            return (InetSocketAddress) activeChannel.getRemoteSocketAddress();
        }

        @Override
        public void sendMessage(BytesReference reference, ActionListener<Void> listener) {
            try {
                synchronized (this) {
                    OutputStream outputStream = new BufferedOutputStream(activeChannel.getOutputStream());
                    reference.writeTo(outputStream);
                    outputStream.flush();
                }
                listener.onResponse(null);
            } catch (IOException e) {
                listener.onFailure(e);
                onException(this, e);
            }
        }
    }


    @Override
    protected void doStart() {
        boolean success = false;
        try {
            if (NetworkService.NETWORK_SERVER.get(settings)) {
                                for (ProfileSettings profileSettings : profileSettings) {
                    bindServer(profileSettings);
                }
            }
            super.doStart();
            success = true;
        } finally {
            if (success == false) {
                doStop();
            }
        }
    }

    @Override
    protected void stopInternal() {
        ThreadPool.terminate(executor, 10, TimeUnit.SECONDS);
        synchronized (openChannels) {
            assert openChannels.isEmpty() : "there are still open channels: " + openChannels;
        }
    }

    @Override
    protected Version getCurrentVersion() {
        return mockVersion;
    }
}

