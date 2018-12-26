

package org.elasticsearch.nio;

import org.elasticsearch.test.ESTestCase;
import org.junit.Before;

import java.io.IOException;
import java.nio.channels.ClosedChannelException;
import java.nio.channels.SelectionKey;
import java.nio.channels.Selector;
import java.security.PrivilegedActionException;
import java.util.Collections;
import java.util.HashSet;

import static org.mockito.Matchers.any;
import static org.mockito.Matchers.same;
import static org.mockito.Mockito.doThrow;
import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.when;

public class AcceptingSelectorTests extends ESTestCase {

    private AcceptingSelector selector;
    private NioServerSocketChannel serverChannel;
    private AcceptorEventHandler eventHandler;
    private TestSelectionKey selectionKey;
    private Selector rawSelector;
    private ServerChannelContext context;

    @Before
    public void setUp() throws Exception {
        super.setUp();

        eventHandler = mock(AcceptorEventHandler.class);
        serverChannel = mock(NioServerSocketChannel.class);

        rawSelector = mock(Selector.class);
        selector = new AcceptingSelector(eventHandler, rawSelector);
        this.selector.setThread();

        context = mock(ServerChannelContext.class);
        selectionKey = new TestSelectionKey(0);
        selectionKey.attach(context);
        when(context.getSelectionKey()).thenReturn(selectionKey);
        when(context.getSelector()).thenReturn(selector);
        when(context.isOpen()).thenReturn(true);
        when(serverChannel.getContext()).thenReturn(context);
    }

    public void testRegisteredChannel() throws IOException {
        selector.scheduleForRegistration(serverChannel);

        selector.preSelect();

        verify(eventHandler).handleRegistration(context);
    }

    public void testClosedChannelWillNotBeRegistered() {
        when(context.isOpen()).thenReturn(false);
        selector.scheduleForRegistration(serverChannel);

        selector.preSelect();

        verify(eventHandler).registrationException(same(context), any(ClosedChannelException.class));
    }

    public void testRegisterChannelFailsDueToException() throws Exception {
        selector.scheduleForRegistration(serverChannel);

        ClosedChannelException closedChannelException = new ClosedChannelException();
        doThrow(closedChannelException).when(eventHandler).handleRegistration(context);

        selector.preSelect();

        verify(eventHandler).registrationException(context, closedChannelException);
    }

    public void testAcceptEvent() throws IOException {
        selectionKey.setReadyOps(SelectionKey.OP_ACCEPT);

        selector.processKey(selectionKey);

        verify(eventHandler).acceptChannel(context);
    }

    public void testAcceptException() throws IOException {
        selectionKey.setReadyOps(SelectionKey.OP_ACCEPT);
        IOException ioException = new IOException();

        doThrow(ioException).when(eventHandler).acceptChannel(context);

        selector.processKey(selectionKey);

        verify(eventHandler).acceptException(context, ioException);
    }

    public void testCleanup() throws IOException {
        selector.scheduleForRegistration(serverChannel);

        selector.preSelect();

        TestSelectionKey key = new TestSelectionKey(0);
        key.attach(context);
        when(rawSelector.keys()).thenReturn(new HashSet<>(Collections.singletonList(key)));

        selector.cleanupAndCloseChannels();

        verify(eventHandler).handleClose(context);
    }
}
