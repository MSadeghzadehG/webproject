

package io.reactivex.parallel;

import static org.junit.Assert.*;

import java.util.List;
import java.util.concurrent.TimeUnit;

import org.junit.Test;

import io.reactivex.*;
import io.reactivex.exceptions.TestException;
import io.reactivex.functions.*;
import io.reactivex.internal.functions.Functions;
import io.reactivex.plugins.RxJavaPlugins;
import io.reactivex.schedulers.Schedulers;

public class ParallelMapTest {

    @Test
    public void subscriberCount() {
        ParallelFlowableTest.checkSubscriberCount(Flowable.range(1, 5).parallel()
        .map(Functions.identity()));
    }

    @Test
    public void doubleFilter() {
        Flowable.range(1, 10)
        .parallel()
        .map(Functions.<Integer>identity())
        .filter(new Predicate<Integer>() {
            @Override
            public boolean test(Integer v) throws Exception {
                return v % 2 == 0;
            }
        })
        .filter(new Predicate<Integer>() {
            @Override
            public boolean test(Integer v) throws Exception {
                return v % 3 == 0;
            }
        })
        .sequential()
        .test()
        .assertResult(6);
    }

    @Test
    public void doubleFilterAsync() {
        Flowable.range(1, 10)
        .parallel()
        .runOn(Schedulers.computation())
        .map(Functions.<Integer>identity())
        .filter(new Predicate<Integer>() {
            @Override
            public boolean test(Integer v) throws Exception {
                return v % 2 == 0;
            }
        })
        .filter(new Predicate<Integer>() {
            @Override
            public boolean test(Integer v) throws Exception {
                return v % 3 == 0;
            }
        })
        .sequential()
        .test()
        .awaitDone(5, TimeUnit.SECONDS)
        .assertResult(6);
    }

    @Test
    public void doubleError() {
        List<Throwable> errors = TestHelper.trackPluginErrors();
        try {
            new ParallelInvalid()
            .map(Functions.<Object>identity())
            .sequential()
            .test()
            .assertFailure(TestException.class);

            assertFalse(errors.isEmpty());
            for (Throwable ex : errors) {
                assertTrue(ex.toString(), ex.getCause() instanceof TestException);
            }
        } finally {
            RxJavaPlugins.reset();
        }
    }

    @Test
    public void doubleError2() {
        List<Throwable> errors = TestHelper.trackPluginErrors();
        try {
            new ParallelInvalid()
            .map(Functions.<Object>identity())
            .filter(Functions.alwaysTrue())
            .sequential()
            .test()
            .assertFailure(TestException.class);

            assertFalse(errors.isEmpty());
            for (Throwable ex : errors) {
                assertTrue(ex.toString(), ex.getCause() instanceof TestException);
            }
        } finally {
            RxJavaPlugins.reset();
        }
    }

    @Test
    public void error() {
        Flowable.error(new TestException())
        .parallel()
        .map(Functions.<Object>identity())
        .sequential()
        .test()
        .assertFailure(TestException.class);
    }

    @Test
    public void mapCrash() {
        Flowable.just(1)
        .parallel()
        .map(new Function<Integer, Object>() {
            @Override
            public Object apply(Integer v) throws Exception {
                throw new TestException();
            }
        })
        .sequential()
        .test()
        .assertFailure(TestException.class);
    }

    @Test
    public void mapCrashConditional() {
        Flowable.just(1)
        .parallel()
        .map(new Function<Integer, Object>() {
            @Override
            public Object apply(Integer v) throws Exception {
                throw new TestException();
            }
        })
        .filter(Functions.alwaysTrue())
        .sequential()
        .test()
        .assertFailure(TestException.class);
    }

    @Test
    public void mapCrashConditional2() {
        Flowable.just(1)
        .parallel()
        .runOn(Schedulers.computation())
        .map(new Function<Integer, Object>() {
            @Override
            public Object apply(Integer v) throws Exception {
                throw new TestException();
            }
        })
        .filter(Functions.alwaysTrue())
        .sequential()
        .test()
        .awaitDone(5, TimeUnit.SECONDS)
        .assertFailure(TestException.class);
    }
}
