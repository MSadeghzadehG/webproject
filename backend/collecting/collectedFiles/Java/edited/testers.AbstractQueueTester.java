

package com.google.common.collect.testing.testers;

import com.google.common.annotations.GwtCompatible;
import com.google.common.collect.testing.AbstractCollectionTester;
import java.util.Queue;
import org.junit.Ignore;


@GwtCompatible
@Ignore public class AbstractQueueTester<E> extends AbstractCollectionTester<E> {
  protected final Queue<E> getQueue() {
    return (Queue<E>) collection;
  }
}
