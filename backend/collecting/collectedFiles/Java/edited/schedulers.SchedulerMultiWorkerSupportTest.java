

package io.reactivex.internal.schedulers;

import static org.junit.Assert.*;

import java.util.*;
import java.util.concurrent.*;

import org.junit.Test;

import io.reactivex.Scheduler.Worker;
import io.reactivex.TestHelper;
import io.reactivex.disposables.CompositeDisposable;
import io.reactivex.internal.schedulers.SchedulerMultiWorkerSupport.WorkerCallback;
import io.reactivex.schedulers.Schedulers;

public class SchedulerMultiWorkerSupportTest {

    final int max = ComputationScheduler.MAX_THREADS;

    @Test
    public void moreThanMaxWorkers() {
        final List<Worker> list = new ArrayList<Worker>();

        SchedulerMultiWorkerSupport mws = (SchedulerMultiWorkerSupport)Schedulers.computation();

        mws.createWorkers(max * 2, new WorkerCallback() {
            @Override
            public void onWorker(int i, Worker w) {
                list.add(w);
            }
        });

        assertEquals(max * 2, list.size());
    }

    @Test
    public void getShutdownWorkers() {
        final List<Worker> list = new ArrayList<Worker>();

        ComputationScheduler.NONE.createWorkers(max * 2, new WorkerCallback() {
            @Override
            public void onWorker(int i, Worker w) {
                list.add(w);
            }
        });

        assertEquals(max * 2, list.size());

        for (Worker w : list) {
            assertEquals(ComputationScheduler.SHUTDOWN_WORKER, w);
        }
    }

    @Test
    public void distinctThreads() throws Exception {
        for (int i = 0; i < TestHelper.RACE_DEFAULT_LOOPS; i++) {

            final CompositeDisposable composite = new CompositeDisposable();

            try {
                final CountDownLatch cdl = new CountDownLatch(max * 2);

                final Set<String> threads1 = Collections.synchronizedSet(new HashSet<String>());

                final Set<String> threads2 = Collections.synchronizedSet(new HashSet<String>());

                Runnable parallel1 = new Runnable() {
                    @Override
                    public void run() {
                        final List<Worker> list1 = new ArrayList<Worker>();

                        SchedulerMultiWorkerSupport mws = (SchedulerMultiWorkerSupport)Schedulers.computation();

                        mws.createWorkers(max, new WorkerCallback() {
                            @Override
                            public void onWorker(int i, Worker w) {
                                list1.add(w);
                                composite.add(w);
                            }
                        });

                        Runnable run = new Runnable() {
                            @Override
                            public void run() {
                                threads1.add(Thread.currentThread().getName());
                                cdl.countDown();
                            }
                        };

                        for (Worker w : list1) {
                            w.schedule(run);
                        }
                    }
                };

                Runnable parallel2 = new Runnable() {
                    @Override
                    public void run() {
                        final List<Worker> list2 = new ArrayList<Worker>();

                        SchedulerMultiWorkerSupport mws = (SchedulerMultiWorkerSupport)Schedulers.computation();

                        mws.createWorkers(max, new WorkerCallback() {
                            @Override
                            public void onWorker(int i, Worker w) {
                                list2.add(w);
                                composite.add(w);
                            }
                        });

                        Runnable run = new Runnable() {
                            @Override
                            public void run() {
                                threads2.add(Thread.currentThread().getName());
                                cdl.countDown();
                            }
                        };

                        for (Worker w : list2) {
                            w.schedule(run);
                        }
                    }
                };

                TestHelper.race(parallel1, parallel2);

                assertTrue(cdl.await(5, TimeUnit.SECONDS));

                assertEquals(threads1.toString(), max, threads1.size());
                assertEquals(threads2.toString(), max, threads2.size());
            } finally {
                composite.dispose();
            }
        }
    }
}