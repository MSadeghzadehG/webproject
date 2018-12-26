

package io.reactivex.internal.schedulers;

import java.util.List;

import org.junit.Test;

import io.reactivex.TestHelper;
import io.reactivex.exceptions.TestException;
import io.reactivex.plugins.RxJavaPlugins;

public class ScheduledDirectPeriodicTaskTest {

    @Test
    public void runnableThrows() {
        List<Throwable> errors = TestHelper.trackPluginErrors();
        try {
            ScheduledDirectPeriodicTask task = new ScheduledDirectPeriodicTask(new Runnable() {
                @Override
                public void run() {
                    throw new TestException();
                }
            });

            task.run();

            TestHelper.assertUndeliverable(errors, 0, TestException.class);
        } finally {
            RxJavaPlugins.reset();
        }
    }
}
