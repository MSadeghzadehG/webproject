

package org.elasticsearch.nio;

import java.nio.channels.SelectableChannel;
import java.nio.channels.SelectionKey;
import java.nio.channels.Selector;
import java.nio.channels.spi.AbstractSelectionKey;

public class TestSelectionKey extends AbstractSelectionKey {

    private int ops = 0;
    private int readyOps;

    public TestSelectionKey(int ops) {
        this.ops = ops;
    }

    @Override
    public SelectableChannel channel() {
        return null;
    }

    @Override
    public Selector selector() {
        return null;
    }

    @Override
    public int interestOps() {
        return ops;
    }

    @Override
    public SelectionKey interestOps(int ops) {
        this.ops = ops;
        return this;
    }

    @Override
    public int readyOps() {
        return readyOps;
    }

    public void setReadyOps(int readyOps) {
        this.readyOps = readyOps;
    }
}
