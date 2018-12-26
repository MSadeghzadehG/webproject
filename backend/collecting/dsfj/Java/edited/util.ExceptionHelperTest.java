

package io.reactivex.internal.util;

import static org.junit.Assert.assertTrue;

import java.util.concurrent.atomic.AtomicReference;

import org.junit.Test;

import io.reactivex.TestHelper;
import io.reactivex.exceptions.TestException;

public class ExceptionHelperTest {
    @Test
    public void utilityClass() {
        TestHelper.checkUtilityClass(ExceptionHelper.class);
    }

    @Test
    public void addRace() {
        for (int i = 0; i < TestHelper.RACE_DEFAULT_LOOPS; i++) {

            final AtomicReference<Throwable> error = new AtomicReference<Throwable>();

            final TestException ex = new TestException();

            Runnable r = new Runnable() {
                @Override
                public void run() {
                    assertTrue(ExceptionHelper.addThrowable(error, ex));
                }
            };

            TestHelper.race(r, r);
        }
    }

    @Test(expected = InternalError.class)
    public void throwIfThrowable() throws Exception {
        ExceptionHelper.<Exception>throwIfThrowable(new InternalError());
    }
}
