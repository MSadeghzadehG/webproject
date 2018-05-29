

package io.reactivex.xmapz;

import java.util.Arrays;
import java.util.concurrent.TimeUnit;

import org.openjdk.jmh.annotations.*;
import org.openjdk.jmh.infra.Blackhole;

import io.reactivex.*;
import io.reactivex.functions.Function;

@BenchmarkMode(Mode.Throughput)
@Warmup(iterations = 5)
@Measurement(iterations = 5, time = 1, timeUnit = TimeUnit.SECONDS)
@OutputTimeUnit(TimeUnit.SECONDS)
@Fork(value = 1)
@State(Scope.Thread)
public class ObservableFlatMapMaybePerf {
    @Param({ "1", "10", "100", "1000", "10000", "100000", "1000000" })
    public int count;

    Observable<Integer> observableConvert;

    Observable<Integer> observableDedicated;

    Observable<Integer> observablePlain;

    @Setup
    public void setup() {
        Integer[] sourceArray = new Integer[count];
        Arrays.fill(sourceArray, 777);

        Observable<Integer> source = Observable.fromArray(sourceArray);

        observablePlain = source.flatMap(new Function<Integer, Observable<? extends Integer>>() {
            @Override
            public Observable<? extends Integer> apply(Integer v)
                    throws Exception {
                return Observable.just(v);
            }
        });

        observableConvert = source.flatMap(new Function<Integer, Observable<? extends Integer>>() {
            @Override
            public Observable<? extends Integer> apply(Integer v)
                    throws Exception {
                return Maybe.just(v).toObservable();
            }
        });

        observableDedicated = source.flatMapMaybe(new Function<Integer, Maybe<? extends Integer>>() {
            @Override
            public Maybe<? extends Integer> apply(Integer v)
                    throws Exception {
                return Maybe.just(v);
            }
        });
    }

    @Benchmark
    public Object observablePlain(Blackhole bh) {
        return observablePlain.subscribeWith(new PerfConsumer(bh));
    }

    @Benchmark
    public Object observableConvert(Blackhole bh) {
        return observableConvert.subscribeWith(new PerfConsumer(bh));
    }

    @Benchmark
    public Object observableDedicated(Blackhole bh) {
        return observableDedicated.subscribeWith(new PerfConsumer(bh));
    }
}