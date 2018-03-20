

package org.elasticsearch.transport.netty4;

import io.netty.buffer.ByteBuf;
import io.netty.buffer.CompositeByteBuf;
import io.netty.buffer.Unpooled;
import io.netty.channel.Channel;
import io.netty.channel.ChannelFuture;
import io.netty.util.NettyRuntime;
import io.netty.util.internal.logging.InternalLogger;
import io.netty.util.internal.logging.InternalLoggerFactory;
import org.apache.logging.log4j.Logger;
import org.apache.lucene.util.BytesRef;
import org.apache.lucene.util.BytesRefIterator;
import org.elasticsearch.ExceptionsHelper;
import org.elasticsearch.common.Booleans;
import org.elasticsearch.common.bytes.BytesReference;
import org.elasticsearch.common.logging.ESLoggerFactory;

import java.io.IOException;
import java.util.ArrayList;
import java.util.Collection;
import java.util.List;
import java.util.Locale;
import java.util.Optional;
import java.util.concurrent.atomic.AtomicBoolean;

public class Netty4Utils {

    static {
        InternalLoggerFactory.setDefaultFactory(new InternalLoggerFactory() {

            @Override
            public InternalLogger newInstance(final String name) {
                return new Netty4InternalESLogger(name);
            }

        });
    }

    public static void setup() {

    }

    private static AtomicBoolean isAvailableProcessorsSet = new AtomicBoolean();

    
    public static void setAvailableProcessors(final int availableProcessors) {
                final boolean set = Booleans.parseBoolean(System.getProperty("es.set.netty.runtime.available.processors", "true"));
        if (!set) {
            return;
        }

        
        if (isAvailableProcessorsSet.compareAndSet(false, true)) {
            NettyRuntime.setAvailableProcessors(availableProcessors);
        } else if (availableProcessors != NettyRuntime.availableProcessors()) {
            
            final String message = String.format(
                    Locale.ROOT,
                    "available processors value [%d] did not match current value [%d]",
                    availableProcessors,
                    NettyRuntime.availableProcessors());
            throw new IllegalStateException(message);
        }
    }

    
    public static ByteBuf toByteBuf(final BytesReference reference) {
        if (reference.length() == 0) {
            return Unpooled.EMPTY_BUFFER;
        }
        if (reference instanceof ByteBufBytesReference) {
            return ((ByteBufBytesReference) reference).toByteBuf();
        } else {
            final BytesRefIterator iterator = reference.iterator();
                        final List<ByteBuf> buffers = new ArrayList<>(3);
            try {
                BytesRef slice;
                while ((slice = iterator.next()) != null) {
                    buffers.add(Unpooled.wrappedBuffer(slice.bytes, slice.offset, slice.length));
                }
                final CompositeByteBuf composite = Unpooled.compositeBuffer(buffers.size());
                composite.addComponents(true, buffers);
                return composite;
            } catch (IOException ex) {
                throw new AssertionError("no IO happens here", ex);
            }
        }
    }

    
    public static BytesReference toBytesReference(final ByteBuf buffer) {
        return toBytesReference(buffer, buffer.readableBytes());
    }

    
    static BytesReference toBytesReference(final ByteBuf buffer, final int size) {
        return new ByteBufBytesReference(buffer, size);
    }

    public static void closeChannels(final Collection<Channel> channels) throws IOException {
        IOException closingExceptions = null;
        final List<ChannelFuture> futures = new ArrayList<>();
        for (final Channel channel : channels) {
            try {
                if (channel != null && channel.isOpen()) {
                    futures.add(channel.close());
                }
            } catch (Exception e) {
                if (closingExceptions == null) {
                    closingExceptions = new IOException("failed to close channels");
                }
                closingExceptions.addSuppressed(e);
            }
        }
        for (final ChannelFuture future : futures) {
            future.awaitUninterruptibly();
        }

        if (closingExceptions != null) {
            throw closingExceptions;
        }
    }

    
    public static void maybeDie(final Throwable cause) {
        final Logger logger = ESLoggerFactory.getLogger(Netty4Utils.class);
        final Optional<Error> maybeError = ExceptionsHelper.maybeError(cause, logger);
        if (maybeError.isPresent()) {
            
            try {
                                final String formatted = ExceptionsHelper.formatStackTrace(Thread.currentThread().getStackTrace());
                logger.error("fatal error on the network layer\n{}", formatted);
            } finally {
                new Thread(
                        () -> {
                            throw maybeError.get();
                        })
                        .start();
            }
        }
    }

}
