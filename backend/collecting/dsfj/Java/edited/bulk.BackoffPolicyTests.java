

package org.elasticsearch.action.bulk;

import org.elasticsearch.common.unit.TimeValue;
import org.elasticsearch.test.ESTestCase;

import java.util.Iterator;
import java.util.NoSuchElementException;
import java.util.concurrent.atomic.AtomicInteger;

import static org.elasticsearch.common.unit.TimeValue.timeValueMillis;

public class BackoffPolicyTests extends ESTestCase {
    public void testWrapBackoffPolicy() {
        TimeValue timeValue = timeValueMillis(between(0, Integer.MAX_VALUE));
        int maxNumberOfRetries = between(1, 1000);
        BackoffPolicy policy = BackoffPolicy.constantBackoff(timeValue, maxNumberOfRetries);
        AtomicInteger retries = new AtomicInteger();
        policy = BackoffPolicy.wrap(policy, retries::getAndIncrement);

        int expectedRetries = 0;
        {
                        Iterator<TimeValue> itr = policy.iterator();
            assertEquals(expectedRetries, retries.get());

            while (itr.hasNext()) {
                                assertEquals(expectedRetries, retries.get());
                                itr.next();
                expectedRetries += 1;
                assertEquals(expectedRetries, retries.get());
            }
                        expectThrows(NoSuchElementException.class, () -> itr.next());
            assertEquals(expectedRetries, retries.get());
        }
        {
                        Iterator<TimeValue> itr = policy.iterator();
            itr.next();
            expectedRetries += 1;
            assertEquals(expectedRetries, retries.get());
        }
    }
}
