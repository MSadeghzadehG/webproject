

package io.reactivex;

import java.util.*;
import java.util.concurrent.TimeUnit;

import org.openjdk.jmh.annotations.*;
import org.openjdk.jmh.infra.Blackhole;

import io.reactivex.Flowable;
import io.reactivex.functions.Function;
import io.reactivex.schedulers.Schedulers;

@BenchmarkMode(Mode.Throughput)
@OutputTimeUnit(TimeUnit.SECONDS)
public class OperatorMergePerf {

        @Benchmark
    public void oneStreamOfNthatMergesIn1(final InputMillion input) throws InterruptedException {
        Flowable<Flowable<Integer>> os = Flowable.range(1, input.size)
                .map(new Function<Integer, Flowable<Integer>>() {
                    @Override
                    public Flowable<Integer> apply(Integer v) {
                        return Flowable.just(v);
                    }
                });
        PerfSubscriber o = input.newLatchedObserver();
        Flowable.merge(os).subscribe(o);

        if (input.size == 1) {
            while (o.latch.getCount() != 0) { }
        } else {
            o.latch.await();
        }
    }

        @Benchmark
    public void merge1SyncStreamOfN(final InputMillion input) throws InterruptedException {
        Flowable<Flowable<Integer>> os = Flowable.just(1).map(new Function<Integer, Flowable<Integer>>() {
            @Override
            public Flowable<Integer> apply(Integer i) {
                    return Flowable.range(0, input.size);
            }
        });
        PerfSubscriber o = input.newLatchedObserver();
        Flowable.merge(os).subscribe(o);

        if (input.size == 1) {
            while (o.latch.getCount() != 0) { }
        } else {
            o.latch.await();
        }
    }

    @Benchmark
    public void mergeNSyncStreamsOfN(final InputThousand input) throws InterruptedException {
        Flowable<Flowable<Integer>> os = input.observable.map(new Function<Integer, Flowable<Integer>>() {
            @Override
            public Flowable<Integer> apply(Integer i) {
                    return Flowable.range(0, input.size);
            }
        });
        PerfSubscriber o = input.newLatchedObserver();
        Flowable.merge(os).subscribe(o);
        if (input.size == 1) {
            while (o.latch.getCount() != 0) { }
        } else {
            o.latch.await();
        }
    }

    @Benchmark
    public void mergeNAsyncStreamsOfN(final InputThousand input) throws InterruptedException {
        Flowable<Flowable<Integer>> os = input.observable.map(new Function<Integer, Flowable<Integer>>() {
            @Override
            public Flowable<Integer> apply(Integer i) {
                    return Flowable.range(0, input.size).subscribeOn(Schedulers.computation());
            }
        });
        PerfSubscriber o = input.newLatchedObserver();
        Flowable.merge(os).subscribe(o);
        if (input.size == 1) {
            while (o.latch.getCount() != 0) { }
        } else {
            o.latch.await();
        }
    }

    @Benchmark
    public void mergeTwoAsyncStreamsOfN(final InputThousand input) throws InterruptedException {
        PerfSubscriber o = input.newLatchedObserver();
        Flowable<Integer> ob = Flowable.range(0, input.size).subscribeOn(Schedulers.computation());
        Flowable.merge(ob, ob).subscribe(o);
        if (input.size == 1) {
            while (o.latch.getCount() != 0) { }
        } else {
            o.latch.await();
        }
    }

    @Benchmark
    public void mergeNSyncStreamsOf1(final InputForMergeN input) throws InterruptedException {
        PerfSubscriber o = input.newLatchedObserver();
        Flowable.merge(input.observables).subscribe(o);
        if (input.size == 1) {
            while (o.latch.getCount() != 0) { }
        } else {
            o.latch.await();
        }
    }

    @State(Scope.Thread)
    public static class InputForMergeN {
        @Param({ "1", "100", "1000" })
                public int size;

        private Blackhole bh;
        List<Flowable<Integer>> observables;

        @Setup
        public void setup(final Blackhole bh) {
            this.bh = bh;
            observables = new ArrayList<Flowable<Integer>>();
            for (int i = 0; i < size; i++) {
                observables.add(Flowable.just(i));
            }
        }

        public PerfSubscriber newLatchedObserver() {
            return new PerfSubscriber(bh);
        }
    }

    @State(Scope.Thread)
    public static class InputMillion extends InputWithIncrementingInteger {

        @Param({ "1", "1000", "1000000" })
                public int size;

        @Override
        public int getSize() {
            return size;
        }

    }

    @State(Scope.Thread)
    public static class InputThousand extends InputWithIncrementingInteger {

        @Param({ "1", "1000" })
                public int size;

        @Override
        public int getSize() {
            return size;
        }

    }
}
