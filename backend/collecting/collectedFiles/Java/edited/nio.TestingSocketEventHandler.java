

package org.elasticsearch.transport.nio;

import org.apache.logging.log4j.Logger;
import org.elasticsearch.nio.SocketChannelContext;
import org.elasticsearch.nio.SocketEventHandler;

import java.io.IOException;
import java.util.Collections;
import java.util.Set;
import java.util.WeakHashMap;

public class TestingSocketEventHandler extends SocketEventHandler {

    public TestingSocketEventHandler(Logger logger) {
        super(logger);
    }

    private Set<SocketChannelContext> hasConnectedMap = Collections.newSetFromMap(new WeakHashMap<>());

    public void handleConnect(SocketChannelContext context) throws IOException {
        assert hasConnectedMap.contains(context) == false : "handleConnect should only be called is a channel is not yet connected";
        super.handleConnect(context);
        if (context.isConnectComplete()) {
            hasConnectedMap.add(context);
        }
    }

    private Set<SocketChannelContext> hasConnectExceptionMap = Collections.newSetFromMap(new WeakHashMap<>());

    public void connectException(SocketChannelContext context, Exception e) {
        assert hasConnectExceptionMap.contains(context) == false : "connectException should only called at maximum once per channel";
        hasConnectExceptionMap.add(context);
        super.connectException(context, e);
    }
}
