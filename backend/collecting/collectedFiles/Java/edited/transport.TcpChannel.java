

package org.elasticsearch.transport;

import org.elasticsearch.action.ActionFuture;
import org.elasticsearch.action.ActionListener;
import org.elasticsearch.action.support.PlainActionFuture;
import org.elasticsearch.cluster.node.DiscoveryNode;
import org.elasticsearch.common.bytes.BytesReference;
import org.elasticsearch.common.lease.Releasable;
import org.elasticsearch.common.lease.Releasables;
import org.elasticsearch.common.unit.TimeValue;

import java.io.IOException;
import java.net.InetSocketAddress;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.TimeoutException;



public interface TcpChannel extends Releasable {

    
    void close();

    
    String getProfile();

    
    void addCloseListener(ActionListener<Void> listener);


    
    void setSoLinger(int value) throws IOException;


    
    boolean isOpen();

    
    InetSocketAddress getLocalAddress();

    
    InetSocketAddress getRemoteAddress();

    
    void sendMessage(BytesReference reference, ActionListener<Void> listener);

    
    static <C extends TcpChannel> void closeChannel(C channel, boolean blocking) {
        closeChannels(Collections.singletonList(channel), blocking);
    }

    
    static <C extends TcpChannel> void closeChannels(List<C> channels, boolean blocking) {
        if (blocking) {
            ArrayList<ActionFuture<Void>> futures = new ArrayList<>(channels.size());
            for (final C channel : channels) {
                if (channel.isOpen()) {
                    PlainActionFuture<Void> closeFuture = PlainActionFuture.newFuture();
                    channel.addCloseListener(closeFuture);
                    channel.close();
                    futures.add(closeFuture);
                }
            }
            blockOnFutures(futures);
        } else {
            Releasables.close(channels);
        }
    }

    
    static void awaitConnected(DiscoveryNode discoveryNode, List<ActionFuture<Void>> connectionFutures, TimeValue connectTimeout)
        throws ConnectTransportException {
        Exception connectionException = null;
        boolean allConnected = true;

        for (ActionFuture<Void> connectionFuture : connectionFutures) {
            try {
                connectionFuture.get(connectTimeout.getMillis(), TimeUnit.MILLISECONDS);
            } catch (TimeoutException e) {
                allConnected = false;
                break;
            } catch (InterruptedException e) {
                Thread.currentThread().interrupt();
                throw new IllegalStateException(e);
            } catch (ExecutionException e) {
                allConnected = false;
                connectionException = (Exception) e.getCause();
                break;
            }
        }

        if (allConnected == false) {
            if (connectionException == null) {
                throw new ConnectTransportException(discoveryNode, "connect_timeout[" + connectTimeout + "]");
            } else {
                throw new ConnectTransportException(discoveryNode, "connect_exception", connectionException);
            }
        }
    }

    static void blockOnFutures(List<ActionFuture<Void>> futures) {
        for (ActionFuture<Void> future : futures) {
            try {
                future.get();
            } catch (ExecutionException e) {
                                            } catch (InterruptedException e) {
                Thread.currentThread().interrupt();
                throw new IllegalStateException("Future got interrupted", e);
            }
        }
    }
}
