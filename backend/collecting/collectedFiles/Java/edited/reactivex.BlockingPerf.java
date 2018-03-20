

package io.reactivex;

import java.util.Arrays;
import java.util.concurrent.TimeUnit;

import org.openjdk.jmh.annotations.*;

@BenchmarkMode(Mode.Throughput)
@Warmup(iterations = 5)
@Measurement(iterations = 5, time = 5, timeUnit = TimeUnit.SECONDS)
@OutputTimeUnit(TimeUnit.SECONDS)
@Fork(value = 1)
@State(Scope.Thread)
public class BlockingPerf {
    @Param({ "1", "1000", "1000000" })
    public int times;

    Flowable<Integer> flowable;

    Observable<Integer> observable;

    @Setup
    public void setup() {
        Integer[] array = new Integer[times];
        Arrays.fill(array, 777);

        flowable = Flowable.fromArray(array);

        observable = Observable.fromArray(array);
    }

    @Benchmark
    public Object flowableBlockingFirst() {
        return flowable.blockingFirst();
    }

    @Benchmark
    public Object flowableBlockingLast() {
        return flowable.blockingLast();
    }

    @Benchmark
    public Object observableBlockingLast() {
        return observable.blockingLast();
    }

    @Benchmark
    public Object observableBlockingFirst() {
        return observable.blockingFirst();
    }
}
