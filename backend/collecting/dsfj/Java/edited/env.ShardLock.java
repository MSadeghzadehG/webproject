

package org.elasticsearch.env;

import org.elasticsearch.index.shard.ShardId;

import java.io.Closeable;
import java.io.IOException;
import java.util.concurrent.atomic.AtomicBoolean;


public abstract class ShardLock implements Closeable {

    private final ShardId shardId;
    private final AtomicBoolean closed = new AtomicBoolean(false);

    public ShardLock(ShardId id) {
        this.shardId = id;
    }

    
    public final ShardId getShardId() {
        return shardId;
    }

    @Override
    public final void close() {
        if (this.closed.compareAndSet(false, true)) {
           closeInternal();
        }
    }

    protected  abstract void closeInternal();

    
    public final boolean isOpen() {
        return closed.get() == false;
    }

    @Override
    public String toString() {
        return "ShardLock{" +
                "shardId=" + shardId +
                '}';
    }

}
