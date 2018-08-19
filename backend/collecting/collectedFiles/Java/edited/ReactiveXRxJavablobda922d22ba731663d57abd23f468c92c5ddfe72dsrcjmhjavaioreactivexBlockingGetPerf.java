

package io.reactivex;

import java.util.concurrent.TimeUnit;

import org.openjdk.jmh.annotations.*;

@BenchmarkMode(Mode.Throughput)
@Warmup(iterations = 5)
@Measurement(iterations = 5, time = 5, timeUnit = TimeUnit.SECONDS)
@OutputTimeUnit(TimeUnit.SECONDS)
@Fork(value = 1)
@State(Scope.Thread)
public class BlockingGetPerf {
    Flowable<Integer> flowable;

    Observable<Integer> observable;

    Single<Integer> single;

    Maybe<Integer> maybe;

    Completable completable;

    @Setup
    public void setup() {
        flowable = Flowable.just(1);

        observable = Observable.just(1);

        single = Single.just(1);

        maybe = Maybe.just(1);

        completable = Completable.complete();
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

    @Benchmark
    public Object single() {
        return single.blockingGet();
    }

    @Benchmark
    public Object maybe() {
        return maybe.blockingGet();
    }

    @Benchmark
    public Object completable() {
        return completable.blockingGet();
    }
}
