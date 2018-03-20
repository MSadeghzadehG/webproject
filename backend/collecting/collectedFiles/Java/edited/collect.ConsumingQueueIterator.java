

package com.google.common.collect;

import static com.google.common.base.Preconditions.checkNotNull;

import com.google.common.annotations.GwtCompatible;
import java.util.ArrayDeque;
import java.util.Collections;
import java.util.Queue;


@GwtCompatible
class ConsumingQueueIterator<T> extends AbstractIterator<T> {
  private final Queue<T> queue;

  ConsumingQueueIterator(T... elements) {
    this.queue = new ArrayDeque<T>(elements.length);
    Collections.addAll(queue, elements);
  }

  ConsumingQueueIterator(Queue<T> queue) {
    this.queue = checkNotNull(queue);
  }

  @Override
  public T computeNext() {
    return queue.isEmpty() ? endOfData() : queue.remove();
  }
}
