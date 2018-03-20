

package io.reactivex;

import java.util.Arrays;
import java.util.concurrent.TimeUnit;

import org.openjdk.jmh.annotations.*;
import org.openjdk.jmh.infra.Blackhole;

import io.reactivex.functions.Function;

@BenchmarkMode(Mode.Throughput)
@Warmup(iterations = 5)
@Measurement(iterations = 5, time = 5, timeUnit = TimeUnit.SECONDS)
@OutputTimeUnit(TimeUnit.SECONDS)
@Fork(value = 1)
@State(Scope.Thread)
public class ObservableFlatMapPerf {
    @Param({ "1", "10", "100", "1000", "10000", "100000", "1000000" })
    public int count;

    Observable<Integer> source;

    @Setup
    public void setup() {
        int d = 1000000 / count;

        Integer[] mainArray = new Integer[count];
        Integer[] innerArray = new Integer[d];

        Arrays.fill(mainArray, 777);
        Arrays.fill(innerArray, 777);

        Observable<Integer> outer = Observable.fromArray(mainArray);
        final Observable<Integer> inner = Observable.fromArray(innerArray);

        source = outer.flatMap(new Function<Integer, Observable<Integer>>() {
            @Override
            public Observable<Integer> apply(Integer t) {
                return inner;
            }
        });
    }

    @Benchmark
    public void flatMapXRange(Blackhole bh) {
        source.subscribe(new PerfObserver(bh));
    }
}
