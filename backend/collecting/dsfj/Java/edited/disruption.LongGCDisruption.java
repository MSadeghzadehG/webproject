

package org.elasticsearch.test.disruption;

import org.elasticsearch.common.Nullable;
import org.elasticsearch.common.SuppressForbidden;
import org.elasticsearch.common.unit.TimeValue;
import org.elasticsearch.common.util.concurrent.AbstractRunnable;
import org.elasticsearch.test.InternalTestCluster;

import java.lang.management.ManagementFactory;
import java.lang.management.ThreadInfo;
import java.lang.management.ThreadMXBean;
import java.util.Arrays;
import java.util.Random;
import java.util.Set;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.atomic.AtomicReference;
import java.util.regex.Pattern;
import java.util.stream.Collectors;


public class LongGCDisruption extends SingleNodeDisruption {

    private static final Pattern[] unsafeClasses = new Pattern[]{
                Pattern.compile("logging\\.log4j"),
                Pattern.compile("java\\.lang\\.SecurityManager"),
                Pattern.compile("java\\.security\\.SecureRandom")
    };

    private static final ThreadMXBean threadBean = ManagementFactory.getThreadMXBean();

    protected final String disruptedNode;
    private Set<Thread> suspendedThreads;
    private Thread blockDetectionThread;

    public LongGCDisruption(Random random, String disruptedNode) {
        super(random);
        this.disruptedNode = disruptedNode;
    }

    @Override
    public synchronized void startDisrupting() {
        if (suspendedThreads == null) {
            boolean success = false;
            try {
                suspendedThreads = ConcurrentHashMap.newKeySet();

                final String currentThreadName = Thread.currentThread().getName();
                assert isDisruptedNodeThread(currentThreadName) == false :
                    "current thread match pattern. thread name: " + currentThreadName + ", node: " + disruptedNode;
                                                                final AtomicReference<Exception> suspendingError = new AtomicReference<>();
                final Thread suspendingThread = new Thread(new AbstractRunnable() {
                    @Override
                    public void onFailure(Exception e) {
                        suspendingError.set(e);
                    }

                    @Override
                    protected void doRun() throws Exception {
                                                while (suspendThreads(suspendedThreads)) {
                            if (Thread.interrupted()) {
                                return;
                            }
                        }
                    }
                });
                suspendingThread.setName(currentThreadName + "[LongGCDisruption][threadSuspender]");
                suspendingThread.start();
                try {
                    suspendingThread.join(getSuspendingTimeoutInMillis());
                } catch (InterruptedException e) {
                    suspendingThread.interrupt();                     throw new RuntimeException(e);
                }
                if (suspendingError.get() != null) {
                    throw new RuntimeException("unknown error while suspending threads", suspendingError.get());
                }
                if (suspendingThread.isAlive()) {
                    logger.warn("failed to suspend node [{}]'s threads within [{}] millis. Suspending thread stack trace:\n {}"
                        , disruptedNode, getSuspendingTimeoutInMillis(), stackTrace(suspendingThread.getStackTrace()));
                    suspendingThread.interrupt();                     try {
                        
                        suspendingThread.join();
                    } catch (InterruptedException e) {
                        throw new RuntimeException(e);
                    }
                    throw new RuntimeException("suspending node threads took too long");
                }
                                                if (isBlockDetectionSupported()) {
                    blockDetectionThread = new Thread(new AbstractRunnable() {
                        @Override
                        public void onFailure(Exception e) {
                            if (e instanceof InterruptedException == false) {
                                throw new AssertionError("unexpected exception in blockDetectionThread", e);
                            }
                        }

                        @Override
                        protected void doRun() throws Exception {
                            while (Thread.currentThread().isInterrupted() == false) {
                                ThreadInfo[] threadInfos = threadBean.dumpAllThreads(true, true);
                                for (ThreadInfo threadInfo : threadInfos) {
                                    if (isDisruptedNodeThread(threadInfo.getThreadName()) == false &&
                                        threadInfo.getLockOwnerName() != null &&
                                        isDisruptedNodeThread(threadInfo.getLockOwnerName())) {

                                                                                ThreadInfo blockingThreadInfo = null;
                                        for (ThreadInfo otherThreadInfo : threadInfos) {
                                            if (otherThreadInfo.getThreadId() == threadInfo.getLockOwnerId()) {
                                                blockingThreadInfo = otherThreadInfo;
                                                break;
                                            }
                                        }
                                        onBlockDetected(threadInfo, blockingThreadInfo);
                                    }
                                }
                                Thread.sleep(getBlockDetectionIntervalInMillis());
                            }
                        }
                    });
                    blockDetectionThread.setName(currentThreadName + "[LongGCDisruption][blockDetection]");
                    blockDetectionThread.start();
                }
                success = true;
            } finally {
                if (success == false) {
                    stopBlockDetection();
                                        resumeThreads(suspendedThreads);
                    suspendedThreads = null;
                }
            }
        } else {
            throw new IllegalStateException("can't disrupt twice, call stopDisrupting() first");
        }
    }

    public boolean isDisruptedNodeThread(String threadName) {
        return threadName.contains("[" + disruptedNode + "]");
    }

    private String stackTrace(StackTraceElement[] stackTraceElements) {
        return Arrays.stream(stackTraceElements).map(Object::toString).collect(Collectors.joining("\n"));
    }

    @Override
    public synchronized void stopDisrupting() {
        stopBlockDetection();
        if (suspendedThreads != null) {
            resumeThreads(suspendedThreads);
            suspendedThreads = null;
        }
    }

    private void stopBlockDetection() {
        if (blockDetectionThread != null) {
            try {
                blockDetectionThread.interrupt();                 blockDetectionThread.join(getSuspendingTimeoutInMillis());
            } catch (InterruptedException e) {
                throw new RuntimeException(e);
            }
            blockDetectionThread = null;
        }
    }

    @Override
    public void removeAndEnsureHealthy(InternalTestCluster cluster) {
        removeFromCluster(cluster);
        ensureNodeCount(cluster);
    }

    @Override
    public TimeValue expectedTimeToHeal() {
        return TimeValue.timeValueMillis(0);
    }

    
    @SuppressWarnings("deprecation")     @SuppressForbidden(reason = "suspends/resumes threads intentionally")
    protected boolean suspendThreads(Set<Thread> nodeThreads) {
        Thread[] allThreads = null;
        while (allThreads == null) {
            allThreads = new Thread[Thread.activeCount()];
            if (Thread.enumerate(allThreads) > allThreads.length) {
                                allThreads = null;
            }
        }
        boolean liveThreadsFound = false;
        for (Thread thread : allThreads) {
            if (thread == null) {
                continue;
            }
            String threadName = thread.getName();
            if (isDisruptedNodeThread(threadName)) {
                if (thread.isAlive() && nodeThreads.add(thread)) {
                    liveThreadsFound = true;
                    logger.trace("suspending thread [{}]", threadName);
                                        boolean safe = false;
                    try {
                        
                        boolean definitelySafe = true;
                        thread.suspend();
                                                safe:
                        for (StackTraceElement stackElement : thread.getStackTrace()) {
                            String className = stackElement.getClassName();
                            for (Pattern unsafePattern : getUnsafeClasses()) {
                                if (unsafePattern.matcher(className).find()) {
                                                                        definitelySafe = false;
                                    break safe;
                                }
                            }
                        }
                        safe = definitelySafe;
                    } finally {
                        if (!safe) {
                            
                            thread.resume();
                            logger.trace("resumed thread [{}] as it is in a critical section", threadName);
                            nodeThreads.remove(thread);
                        }
                    }
                }
            }
        }
        return liveThreadsFound;
    }

        protected Pattern[] getUnsafeClasses() {
        return unsafeClasses;
    }

        protected long getSuspendingTimeoutInMillis() {
        return TimeValue.timeValueSeconds(30).getMillis();
    }

    public boolean isBlockDetectionSupported() {
        return threadBean.isObjectMonitorUsageSupported() && threadBean.isSynchronizerUsageSupported();
    }

        protected long getBlockDetectionIntervalInMillis() {
        return 3000L;
    }

        protected void onBlockDetected(ThreadInfo blockedThread, @Nullable ThreadInfo blockingThread) {
        String blockedThreadStackTrace = stackTrace(blockedThread.getStackTrace());
        String blockingThreadStackTrace = blockingThread != null ?
            stackTrace(blockingThread.getStackTrace()) : "not available";
        throw new AssertionError("Thread [" + blockedThread.getThreadName() + "] is blocked waiting on the resource [" +
            blockedThread.getLockInfo() + "] held by the suspended thread [" + blockedThread.getLockOwnerName() +
            "] of the disrupted node [" + disruptedNode + "].\n" +
            "Please add this occurrence to the unsafeClasses list in [" + LongGCDisruption.class.getName() + "].\n" +
            "Stack trace of blocked thread: " + blockedThreadStackTrace + "\n" +
            "Stack trace of blocking thread: " + blockingThreadStackTrace);
    }

    @SuppressWarnings("deprecation")     @SuppressForbidden(reason = "suspends/resumes threads intentionally")
    protected void resumeThreads(Set<Thread> threads) {
        for (Thread thread : threads) {
            thread.resume();
        }
    }
}
