

package jenkins.util;

import java.util.concurrent.CancellationException;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.Future;
import java.util.concurrent.RejectedExecutionHandler;
import java.util.concurrent.ScheduledThreadPoolExecutor;
import java.util.concurrent.ThreadFactory;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.TimeoutException;
import java.util.logging.Level;
import java.util.logging.Logger;


class ErrorLoggingScheduledThreadPoolExecutor extends ScheduledThreadPoolExecutor {

    private static final Logger LOGGER = Logger.getLogger(ErrorLoggingScheduledThreadPoolExecutor.class.getName());

    ErrorLoggingScheduledThreadPoolExecutor(int corePoolSize) {
        super(corePoolSize);
    }

    ErrorLoggingScheduledThreadPoolExecutor(int corePoolSize, ThreadFactory threadFactory) {
        super(corePoolSize, threadFactory);
    }

    ErrorLoggingScheduledThreadPoolExecutor(int corePoolSize, RejectedExecutionHandler handler) {
        super(corePoolSize, handler);
    }
    
    ErrorLoggingScheduledThreadPoolExecutor(int corePoolSize, ThreadFactory threadFactory, RejectedExecutionHandler handler) {
        super(corePoolSize, threadFactory, handler);
    }
    
    @Override protected void afterExecute(Runnable r, Throwable t) {
        super.afterExecute(r, t);
        if (t == null && r instanceof Future<?>) {
            Future<?> f = (Future<?>) r;
            if (f.isDone()) {                 try {
                    f.get(0, TimeUnit.NANOSECONDS);
                } catch (TimeoutException x) {
                                    } catch (CancellationException x) {
                                    } catch (ExecutionException x) {
                    t = x.getCause();
                } catch (InterruptedException x) {
                    Thread.currentThread().interrupt();
                }
            }
        }
        if (t != null) {
            LOGGER.log(Level.WARNING, "failure in task not wrapped in SafeTimerTask", t);
        }
    }

}
