

package com.google.common.graph;

import static com.google.common.base.Preconditions.checkNotNull;

import com.google.common.collect.AbstractIterator;
import com.google.common.collect.UnmodifiableIterator;
import java.util.AbstractSet;
import java.util.Iterator;
import java.util.Map;
import java.util.Map.Entry;
import org.checkerframework.checker.nullness.compatqual.NullableDecl;


abstract class MultiEdgesConnecting<E> extends AbstractSet<E> {

  private final Map<E, ?> outEdgeToNode;
  private final Object targetNode;

  MultiEdgesConnecting(Map<E, ?> outEdgeToNode, Object targetNode) {
    this.outEdgeToNode = checkNotNull(outEdgeToNode);
    this.targetNode = checkNotNull(targetNode);
  }

  @Override
  public UnmodifiableIterator<E> iterator() {
    final Iterator<? extends Entry<E, ?>> entries = outEdgeToNode.entrySet().iterator();
    return new AbstractIterator<E>() {
      @Override
      protected E computeNext() {
        while (entries.hasNext()) {
          Entry<E, ?> entry = entries.next();
          if (targetNode.equals(entry.getValue())) {
            return entry.getKey();
          }
        }
        return endOfData();
      }
    };
  }

  @Override
  public boolean contains(@NullableDecl Object edge) {
    return targetNode.equals(outEdgeToNode.get(edge));
  }
}
