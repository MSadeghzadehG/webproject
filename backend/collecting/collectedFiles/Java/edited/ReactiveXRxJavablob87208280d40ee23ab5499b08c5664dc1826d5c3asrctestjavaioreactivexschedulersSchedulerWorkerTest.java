

package io.reactivex.schedulers;

import static org.junit.Assert.*;

import java.util.*;
import java.util.concurrent.TimeUnit;

import io.reactivex.annotations.NonNull;
import org.junit.Test;

import io.reactivex.Scheduler;
import io.reactivex.disposables.Disposable;

public class SchedulerWorkerTest {

    static final class CustomDriftScheduler extends Scheduler {
        public volatile long drift;
        @NonNull
        @Override
        public Worker createWorker() {
            final Worker w = Schedulers.computation().createWorker();
            return new Worker() {

                @Override
                public void dispose() {
                    w.dispose();
                }

                @Override
                public boolean isDisposed() {
                    return w.isDisposed();
                }

                @NonNull
                @Override
                public Disposable schedule(@NonNull Runnable action) {
                    return w.schedule(action);
                }

                @NonNull
                @Override
                public Disposable schedule(@NonNull Runnable action, long delayTime, @NonNull TimeUnit unit) {
                    return w.schedule(action, delayTime, unit);
                }

                @Override
                public long now(TimeUnit unit) {
                    return super.now(unit) + unit.convert(drift, TimeUnit.NANOSECONDS);
                }
            };
        }

        @Override
        public long now(@NonNull TimeUnit unit) {
            return super.now(unit) + unit.convert(drift, TimeUnit.NANOSECONDS);
        }
    }

    @Test
    public void testCurrentTimeDriftBackwards() throws Exception {
        CustomDriftScheduler s = new CustomDriftScheduler();

        Scheduler.Worker w = s.createWorker();

        try {
            final List<Long> times = new ArrayList<Long>();

            Disposable d = w.schedulePeriodically(new Runnable() {
                @Override
                public void run() {
                    times.add(System.currentTimeMillis());
                }
            }, 100, 100, TimeUnit.MILLISECONDS);

            Thread.sleep(150);

            s.drift = -TimeUnit.SECONDS.toNanos(1) - Scheduler.clockDriftTolerance();

            Thread.sleep(400);

            d.dispose();

            Thread.sleep(150);

            System.out.println("Runs: " + times.size());

            for (int i = 0; i < times.size() - 1 ; i++) {
                long diff = times.get(i + 1) - times.get(i);
                System.out.println("Diff #" + i + ": " + diff);
                assertTrue("" + i + ":" + diff, diff < 150 && diff > 50);
            }

            assertTrue("Too few invocations: " + times.size(), times.size() > 2);

        } finally {
            w.dispose();
        }

    }

    @Test
    public void testCurrentTimeDriftForwards() throws Exception {
        CustomDriftScheduler s = new CustomDriftScheduler();

        Scheduler.Worker w = s.createWorker();

        try {
            final List<Long> times = new ArrayList<Long>();

            Disposable d = w.schedulePeriodically(new Runnable() {
                @Override
                public void run() {
                    times.add(System.currentTimeMillis());
                }
            }, 100, 100, TimeUnit.MILLISECONDS);

            Thread.sleep(150);

            s.drift = TimeUnit.SECONDS.toNanos(1) + Scheduler.clockDriftTolerance();

            Thread.sleep(400);

            d.dispose();

            Thread.sleep(150);

            System.out.println("Runs: " + times.size());

            assertTrue(times.size() > 2);

            for (int i = 0; i < times.size() - 1 ; i++) {
                long diff = times.get(i + 1) - times.get(i);
                System.out.println("Diff #" + i + ": " + diff);
                assertTrue("Diff out of range: " + diff, diff < 250 && diff > 50);
            }

        } finally {
            w.dispose();
        }

    }
}
