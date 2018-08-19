

package io.reactivex;

import java.util.concurrent.TimeUnit;

import org.openjdk.jmh.annotations.*;
import org.openjdk.jmh.infra.Blackhole;

import io.reactivex.processors.PublishProcessor;
import io.reactivex.subjects.PublishSubject;

@BenchmarkMode(Mode.Throughput)
@Warmup(iterations = 5)
@Measurement(iterations = 5, time = 5, timeUnit = TimeUnit.SECONDS)
@OutputTimeUnit(TimeUnit.SECONDS)
@Fork(value = 1)
@State(Scope.Thread)
public class PublishProcessorPerf {

    PublishProcessor<Integer> unbounded;

    PublishProcessor<Integer> bounded;

    PublishSubject<Integer> subject;

    @Setup
    public void setup(Blackhole bh) {
        unbounded = PublishProcessor.create();
        unbounded.subscribe(new PerfConsumer(bh));

        bounded = PublishProcessor.create();
        bounded.subscribe(new PerfBoundedSubscriber(bh, 1000 * 1000));

        subject = PublishSubject.create();
        subject.subscribe(new PerfConsumer(bh));
    }

    @Benchmark
    public void unbounded1() {
        unbounded.onNext(1);
    }

    @Benchmark
    public void unbounded1k() {
        for (int i = 0; i < 1000; i++) {
            unbounded.onNext(1);
        }
    }

    @Benchmark
    public void unbounded1m() {
        for (int i = 0; i < 1000000; i++) {
            unbounded.onNext(1);
        }
    }

    @Benchmark
    public void bounded1() {
        bounded.onNext(1);
    }


    @Benchmark
    public void bounded1k() {
        for (int i = 0; i < 1000; i++) {
            bounded.onNext(1);
        }
    }

    @Benchmark
    public void bounded1m() {
        for (int i = 0; i < 1000000; i++) {
            bounded.onNext(1);
        }
    }


    @Benchmark
    public void subject1() {
        subject.onNext(1);
    }


    @Benchmark
    public void subject1k() {
        for (int i = 0; i < 1000; i++) {
            subject.onNext(1);
        }
    }

    @Benchmark
    public void subject1m() {
        for (int i = 0; i < 1000000; i++) {
            subject.onNext(1);
        }
    }
}
