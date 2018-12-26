
package com.alibaba.dubbo.common.concurrent;

import com.alibaba.dubbo.common.logger.Logger;
import com.alibaba.dubbo.common.logger.LoggerFactory;
import com.alibaba.dubbo.common.utils.NamedThreadFactory;

import java.util.concurrent.Executor;
import java.util.concurrent.SynchronousQueue;
import java.util.concurrent.ThreadPoolExecutor;
import java.util.concurrent.TimeUnit;


public final class ExecutionList {
        static final Logger logger = LoggerFactory.getLogger(ExecutionList.class.getName());

    
    private RunnableExecutorPair runnables;

    private boolean executed;

    private static final Executor DEFAULT_EXECUTOR = new ThreadPoolExecutor(1, 10, 60000L, TimeUnit.MILLISECONDS, new SynchronousQueue<Runnable>(), new NamedThreadFactory("DubboFutureCallbackDefault", true));

    
    public ExecutionList() {
    }

    
    public void add(Runnable runnable, Executor executor) {
                                if (runnable == null) {
            throw new NullPointerException("Runnable can not be null!");
        }
        if (executor == null) {
            logger.info("Executor for listenablefuture is null, will use default executor!");
            executor = DEFAULT_EXECUTOR;
        }
                                synchronized (this) {
            if (!executed) {
                runnables = new RunnableExecutorPair(runnable, executor, runnables);
                return;
            }
        }
                                        executeListener(runnable, executor);
    }

    
    public void execute() {
                        RunnableExecutorPair list;
        synchronized (this) {
            if (executed) {
                return;
            }
            executed = true;
            list = runnables;
            runnables = null;          }
                                                
                        RunnableExecutorPair reversedList = null;
        while (list != null) {
            RunnableExecutorPair tmp = list;
            list = list.next;
            tmp.next = reversedList;
            reversedList = tmp;
        }
        while (reversedList != null) {
            executeListener(reversedList.runnable, reversedList.executor);
            reversedList = reversedList.next;
        }
    }

    
    private static void executeListener(Runnable runnable, Executor executor) {
        try {
            executor.execute(runnable);
        } catch (RuntimeException e) {
                                                logger.error("RuntimeException while executing runnable "
                    + runnable + " with executor " + executor, e);
        }
    }

    private static final class RunnableExecutorPair {
        final Runnable runnable;
        final Executor executor;
        RunnableExecutorPair next;

        RunnableExecutorPair(Runnable runnable, Executor executor, RunnableExecutorPair next) {
            this.runnable = runnable;
            this.executor = executor;
            this.next = next;
        }
    }
}
