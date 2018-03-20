

package com.google.common.util.concurrent;

import static com.google.common.base.Preconditions.checkNotNull;

import com.google.common.annotations.GwtIncompatible;
import com.google.common.base.Preconditions;
import com.google.common.collect.Queues;
import com.google.errorprone.annotations.concurrent.GuardedBy;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.Queue;
import java.util.concurrent.Executor;
import java.util.logging.Level;
import java.util.logging.Logger;


@GwtIncompatible
final class ListenerCallQueue<L> {
    private static final Logger logger = Logger.getLogger(ListenerCallQueue.class.getName());

    private final List<PerListenerQueue<L>> listeners =
      Collections.synchronizedList(new ArrayList<PerListenerQueue<L>>());

  
  interface Event<L> {
    
    void call(L listener);
  }

  
  public void addListener(L listener, Executor executor) {
    checkNotNull(listener, "listener");
    checkNotNull(executor, "executor");
    listeners.add(new PerListenerQueue<>(listener, executor));
  }

  
  public void enqueue(Event<L> event) {
    enqueueHelper(event, event);
  }

  
  public void enqueue(Event<L> event, String label) {
    enqueueHelper(event, label);
  }

  private void enqueueHelper(Event<L> event, Object label) {
    checkNotNull(event, "event");
    checkNotNull(label, "label");
    synchronized (listeners) {
      for (PerListenerQueue<L> queue : listeners) {
        queue.add(event, label);
      }
    }
  }

  
  public void dispatch() {
        for (int i = 0; i < listeners.size(); i++) {
      listeners.get(i).dispatch();
    }
  }

  
  private static final class PerListenerQueue<L> implements Runnable {
    final L listener;
    final Executor executor;

    @GuardedBy("this")
    final Queue<ListenerCallQueue.Event<L>> waitQueue = Queues.newArrayDeque();

    @GuardedBy("this")
    final Queue<Object> labelQueue = Queues.newArrayDeque();

    @GuardedBy("this")
    boolean isThreadScheduled;

    PerListenerQueue(L listener, Executor executor) {
      this.listener = checkNotNull(listener);
      this.executor = checkNotNull(executor);
    }

    
    synchronized void add(ListenerCallQueue.Event<L> event, Object label) {
      waitQueue.add(event);
      labelQueue.add(label);
    }

    
    void dispatch() {
      boolean scheduleEventRunner = false;
      synchronized (this) {
        if (!isThreadScheduled) {
          isThreadScheduled = true;
          scheduleEventRunner = true;
        }
      }
      if (scheduleEventRunner) {
        try {
          executor.execute(this);
        } catch (RuntimeException e) {
                    synchronized (this) {
            isThreadScheduled = false;
          }
                    logger.log(
              Level.SEVERE,
              "Exception while running callbacks for " + listener + " on " + executor,
              e);
          throw e;
        }
      }
    }

    @Override
    public void run() {
      boolean stillRunning = true;
      try {
        while (true) {
          ListenerCallQueue.Event<L> nextToRun;
          Object nextLabel;
          synchronized (PerListenerQueue.this) {
            Preconditions.checkState(isThreadScheduled);
            nextToRun = waitQueue.poll();
            nextLabel = labelQueue.poll();
            if (nextToRun == null) {
              isThreadScheduled = false;
              stillRunning = false;
              break;
            }
          }

                    try {
            nextToRun.call(listener);
          } catch (RuntimeException e) {
                        logger.log(
                Level.SEVERE,
                "Exception while executing callback: " + listener + " " + nextLabel,
                e);
          }
        }
      } finally {
        if (stillRunning) {
                              synchronized (PerListenerQueue.this) {
            isThreadScheduled = false;
          }
        }
      }
    }
  }
}
