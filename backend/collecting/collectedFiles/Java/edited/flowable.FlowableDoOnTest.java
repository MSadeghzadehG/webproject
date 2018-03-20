

package io.reactivex.flowable;

import static org.junit.Assert.*;

import java.util.concurrent.atomic.*;

import org.junit.Test;

import io.reactivex.*;
import io.reactivex.exceptions.TestException;
import io.reactivex.functions.*;

public class FlowableDoOnTest {

    @Test
    public void testDoOnEach() {
        final AtomicReference<String> r = new AtomicReference<String>();
        String output = Flowable.just("one").doOnNext(new Consumer<String>() {
            @Override
            public void accept(String v) {
                r.set(v);
            }
        }).blockingSingle();

        assertEquals("one", output);
        assertEquals("one", r.get());
    }

    @Test
    public void testDoOnError() {
        final AtomicReference<Throwable> r = new AtomicReference<Throwable>();
        Throwable t = null;
        try {
            Flowable.<String> error(new RuntimeException("an error"))
            .doOnError(new Consumer<Throwable>() {
                @Override
                public void accept(Throwable v) {
                    r.set(v);
                }
            }).blockingSingle();
            fail("expected exception, not a return value");
        } catch (Throwable e) {
            t = e;
        }

        assertNotNull(t);
        assertEquals(t, r.get());
    }

    @Test
    public void testDoOnCompleted() {
        final AtomicBoolean r = new AtomicBoolean();
        String output = Flowable.just("one").doOnComplete(new Action() {
            @Override
            public void run() {
                r.set(true);
            }
        }).blockingSingle();

        assertEquals("one", output);
        assertTrue(r.get());
    }

    @Test
    public void doOnTerminateError() {
        final AtomicBoolean r = new AtomicBoolean();
        Flowable.<String>error(new TestException()).doOnTerminate(new Action() {
            @Override
            public void run() {
                r.set(true);
            }
        })
        .test()
        .assertFailure(TestException.class);
        assertTrue(r.get());
    }

    @Test
    public void doOnTerminateComplete() {
        final AtomicBoolean r = new AtomicBoolean();
        String output = Flowable.just("one").doOnTerminate(new Action() {
            @Override
            public void run() {
                r.set(true);
            }
        }).blockingSingle();

        assertEquals("one", output);
        assertTrue(r.get());

    }
}
