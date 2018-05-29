
package org.elasticsearch.xpack.security.authc.support;

import org.elasticsearch.common.settings.SecureString;
import org.elasticsearch.test.ESTestCase;
import org.elasticsearch.xpack.core.security.authc.support.BCrypt;

import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.Callable;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.Future;
import java.util.concurrent.atomic.AtomicBoolean;

import static org.hamcrest.Matchers.is;


public class BCryptTests extends ESTestCase {
    
    @AwaitsFix(bugUrl = "need a better way to test this")
    public void testUnderLoad() throws Exception {
        final String password = randomAlphaOfLengthBetween(10, 32);
        final String bcrypt = BCrypt.hashpw(new SecureString(password), BCrypt.gensalt());

        ExecutorService threadPoolExecutor = Executors.newFixedThreadPool(100);
        try {
            List<Callable<Boolean>> callables = new ArrayList<>(100);

            final AtomicBoolean failed = new AtomicBoolean(false);
            for (int i = 0; i < 100; i++) {
                callables.add(new Callable<Boolean>() {
                    @Override
                    public Boolean call() throws Exception {
                        for (int i = 0; i < 10000 && !failed.get(); i++) {
                            if (BCrypt.checkpw(new SecureString(password), bcrypt) == false) {
                                failed.set(true);
                                return false;
                            }
                        }
                        return true;
                    }
                });
            }

            List<Future<Boolean>> futures = threadPoolExecutor.invokeAll(callables);
            for (Future<Boolean> future : futures) {
                assertThat(future.get(), is(true));
            }
        } finally {
            threadPoolExecutor.shutdownNow();
        }

    }
}
