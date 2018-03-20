

package org.elasticsearch.nio;

import org.apache.logging.log4j.Logger;
import org.elasticsearch.nio.utils.ExceptionsHelper;

import java.io.IOException;
import java.net.InetSocketAddress;
import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.ThreadFactory;
import java.util.concurrent.atomic.AtomicBoolean;
import java.util.function.BiFunction;
import java.util.function.Function;
import java.util.function.Supplier;
import java.util.stream.Collectors;
import java.util.stream.Stream;


public class NioGroup implements AutoCloseable {


    private final ArrayList<AcceptingSelector> acceptors;
    private final RoundRobinSupplier<AcceptingSelector> acceptorSupplier;

    private final ArrayList<SocketSelector> socketSelectors;
    private final RoundRobinSupplier<SocketSelector> socketSelectorSupplier;

    private final AtomicBoolean isOpen = new AtomicBoolean(true);

    public NioGroup(Logger logger, ThreadFactory acceptorThreadFactory, int acceptorCount,
                    BiFunction<Logger, Supplier<SocketSelector>, AcceptorEventHandler> acceptorEventHandlerFunction,
                    ThreadFactory socketSelectorThreadFactory, int socketSelectorCount,
                    Function<Logger, SocketEventHandler> socketEventHandlerFunction) throws IOException {
        acceptors = new ArrayList<>(acceptorCount);
        socketSelectors = new ArrayList<>(socketSelectorCount);

        try {
            for (int i = 0; i < socketSelectorCount; ++i) {
                SocketSelector selector = new SocketSelector(socketEventHandlerFunction.apply(logger));
                socketSelectors.add(selector);
            }
            startSelectors(socketSelectors, socketSelectorThreadFactory);

            for (int i = 0; i < acceptorCount; ++i) {
                SocketSelector[] childSelectors = this.socketSelectors.toArray(new SocketSelector[this.socketSelectors.size()]);
                Supplier<SocketSelector> selectorSupplier = new RoundRobinSupplier<>(childSelectors);
                AcceptingSelector acceptor = new AcceptingSelector(acceptorEventHandlerFunction.apply(logger, selectorSupplier));
                acceptors.add(acceptor);
            }
            startSelectors(acceptors, acceptorThreadFactory);
        } catch (Exception e) {
            try {
                close();
            } catch (Exception e1) {
                e.addSuppressed(e1);
            }
            throw e;
        }

        socketSelectorSupplier = new RoundRobinSupplier<>(socketSelectors.toArray(new SocketSelector[socketSelectors.size()]));
        acceptorSupplier = new RoundRobinSupplier<>(acceptors.toArray(new AcceptingSelector[acceptors.size()]));
    }

    public <S extends NioServerSocketChannel> S bindServerChannel(InetSocketAddress address, ChannelFactory<S, ?> factory)
        throws IOException {
        ensureOpen();
        if (acceptors.isEmpty()) {
            throw new IllegalArgumentException("There are no acceptors configured. Without acceptors, server channels are not supported.");
        }
        return factory.openNioServerSocketChannel(address, acceptorSupplier);
    }

    public <S extends NioSocketChannel> S openChannel(InetSocketAddress address, ChannelFactory<?, S> factory) throws IOException {
        ensureOpen();
        return factory.openNioChannel(address, socketSelectorSupplier);
    }

    @Override
    public void close() throws IOException {
        if (isOpen.compareAndSet(true, false)) {
            List<ESSelector> toClose = Stream.concat(acceptors.stream(), socketSelectors.stream()).collect(Collectors.toList());
            List<IOException> closingExceptions = new ArrayList<>();
            for (ESSelector selector : toClose) {
                try {
                    selector.close();
                } catch (IOException e) {
                    closingExceptions.add(e);
                }
            }
            ExceptionsHelper.rethrowAndSuppress(closingExceptions);
        }
    }

    private static <S extends ESSelector> void startSelectors(Iterable<S> selectors, ThreadFactory threadFactory) {
        for (ESSelector acceptor : selectors) {
            if (acceptor.isRunning() == false) {
                threadFactory.newThread(acceptor::runLoop).start();
                try {
                    acceptor.isRunningFuture().get();
                } catch (InterruptedException e) {
                    Thread.currentThread().interrupt();
                    throw new IllegalStateException("Interrupted while waiting for selector to start.", e);
                } catch (ExecutionException e) {
                    if (e.getCause() instanceof RuntimeException) {
                        throw (RuntimeException) e.getCause();
                    } else {
                        throw new RuntimeException("Exception during selector start.", e);
                    }
                }
            }
        }
    }

    private void ensureOpen() {
        if (isOpen.get() == false) {
            throw new IllegalStateException("NioGroup is closed.");
        }
    }
}
