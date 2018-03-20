package hudson.util;

import javax.annotation.Nonnull;
import java.util.HashMap;
import java.util.Map;
import java.util.Set;
import java.util.HashSet;
import java.util.concurrent.Executor;
import java.util.concurrent.ExecutorService;


public class SequentialExecutionQueue implements Executor {
    
    private final Map<Runnable,QueueEntry> entries = new HashMap<Runnable,QueueEntry>();
    private ExecutorService executors;

    
    private final Set<QueueEntry> inProgress = new HashSet<QueueEntry>();

    public SequentialExecutionQueue(ExecutorService executors) {
        this.executors = executors;
    }

    
    public synchronized ExecutorService getExecutors() {
        return executors;
    }

    
    public synchronized void setExecutors(ExecutorService svc) {
        ExecutorService old = this.executors;
        this.executors = svc;
                old.shutdown();
    }


    public synchronized void execute(@Nonnull Runnable item) {
        QueueEntry e = entries.get(item);
        if(e==null) {
            e = new QueueEntry(item);
            entries.put(item,e);
            e.submit();
        } else {
            e.queued = true;
        }
    }

    
    public synchronized boolean isStarving(long threshold) {
        long now = System.currentTimeMillis();
        for (QueueEntry e : entries.values())
            if (now-e.submissionTime > threshold)
                return true;
        return false;
    }

    
    public synchronized Set<Runnable> getInProgress() {
        Set<Runnable> items = new HashSet<Runnable>();
        for (QueueEntry entry : inProgress) {
            items.add(entry.item);
        }
        return items;
    }

    private final class QueueEntry implements Runnable {
        private final Runnable item;
        private boolean queued;
        private long submissionTime;

        private QueueEntry(Runnable item) {
            this.item = item;
            this.queued = true;
        }

                private void submit() {
            submissionTime = System.currentTimeMillis();
            executors.submit(this);
        }

        public void run() {
            try {
                synchronized (SequentialExecutionQueue.this) {
                    assert queued;
                    queued = false;
                    inProgress.add(this);
                }
                item.run();
            } finally {
                synchronized (SequentialExecutionQueue.this) {
                    if(queued)
                                                submit();
                    else
                        entries.remove(item);
                    inProgress.remove(this);
                }
            }
        }
    }
}
