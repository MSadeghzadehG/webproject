

package com.google.common.graph;

import static com.google.common.base.Preconditions.checkNotNull;
import static com.google.common.base.Preconditions.checkState;
import static com.google.common.graph.GraphConstants.INNER_CAPACITY;
import static com.google.common.graph.GraphConstants.INNER_LOAD_FACTOR;
import static com.google.common.graph.Graphs.checkNonNegative;
import static com.google.common.graph.Graphs.checkPositive;

import com.google.common.collect.AbstractIterator;
import com.google.common.collect.ImmutableMap;
import com.google.common.collect.UnmodifiableIterator;
import java.util.AbstractSet;
import java.util.Collections;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;
import java.util.Map.Entry;
import java.util.Set;
import org.checkerframework.checker.nullness.compatqual.NullableDecl;


final class DirectedGraphConnections<N, V> implements GraphConnections<N, V> {
  
  private static final class PredAndSucc {
    private final Object successorValue;

    PredAndSucc(Object successorValue) {
      this.successorValue = successorValue;
    }
  }

  private static final Object PRED = new Object();

      private final Map<N, Object> adjacentNodeValues;

  private int predecessorCount;
  private int successorCount;

  private DirectedGraphConnections(
      Map<N, Object> adjacentNodeValues, int predecessorCount, int successorCount) {
    this.adjacentNodeValues = checkNotNull(adjacentNodeValues);
    this.predecessorCount = checkNonNegative(predecessorCount);
    this.successorCount = checkNonNegative(successorCount);
    checkState(
        predecessorCount <= adjacentNodeValues.size()
            && successorCount <= adjacentNodeValues.size());
  }

  static <N, V> DirectedGraphConnections<N, V> of() {
        int initialCapacity = INNER_CAPACITY * 2;
    return new DirectedGraphConnections<>(
        new HashMap<N, Object>(initialCapacity, INNER_LOAD_FACTOR), 0, 0);
  }

  static <N, V> DirectedGraphConnections<N, V> ofImmutable(
      Set<N> predecessors, Map<N, V> successorValues) {
    Map<N, Object> adjacentNodeValues = new HashMap<>();
    adjacentNodeValues.putAll(successorValues);
    for (N predecessor : predecessors) {
      Object value = adjacentNodeValues.put(predecessor, PRED);
      if (value != null) {
        adjacentNodeValues.put(predecessor, new PredAndSucc(value));
      }
    }
    return new DirectedGraphConnections<>(
        ImmutableMap.copyOf(adjacentNodeValues), predecessors.size(), successorValues.size());
  }

  @Override
  public Set<N> adjacentNodes() {
    return Collections.unmodifiableSet(adjacentNodeValues.keySet());
  }

  @Override
  public Set<N> predecessors() {
    return new AbstractSet<N>() {
      @Override
      public UnmodifiableIterator<N> iterator() {
        final Iterator<Entry<N, Object>> entries = adjacentNodeValues.entrySet().iterator();
        return new AbstractIterator<N>() {
          @Override
          protected N computeNext() {
            while (entries.hasNext()) {
              Entry<N, Object> entry = entries.next();
              if (isPredecessor(entry.getValue())) {
                return entry.getKey();
              }
            }
            return endOfData();
          }
        };
      }

      @Override
      public int size() {
        return predecessorCount;
      }

      @Override
      public boolean contains(@NullableDecl Object obj) {
        return isPredecessor(adjacentNodeValues.get(obj));
      }
    };
  }

  @Override
  public Set<N> successors() {
    return new AbstractSet<N>() {
      @Override
      public UnmodifiableIterator<N> iterator() {
        final Iterator<Entry<N, Object>> entries = adjacentNodeValues.entrySet().iterator();
        return new AbstractIterator<N>() {
          @Override
          protected N computeNext() {
            while (entries.hasNext()) {
              Entry<N, Object> entry = entries.next();
              if (isSuccessor(entry.getValue())) {
                return entry.getKey();
              }
            }
            return endOfData();
          }
        };
      }

      @Override
      public int size() {
        return successorCount;
      }

      @Override
      public boolean contains(@NullableDecl Object obj) {
        return isSuccessor(adjacentNodeValues.get(obj));
      }
    };
  }

  @SuppressWarnings("unchecked")
  @Override
  public V value(N node) {
    Object value = adjacentNodeValues.get(node);
    if (value == PRED) {
      return null;
    }
    if (value instanceof PredAndSucc) {
      return (V) ((PredAndSucc) value).successorValue;
    }
    return (V) value;
  }

  @SuppressWarnings("unchecked")
  @Override
  public void removePredecessor(N node) {
    Object previousValue = adjacentNodeValues.get(node);
    if (previousValue == PRED) {
      adjacentNodeValues.remove(node);
      checkNonNegative(--predecessorCount);
    } else if (previousValue instanceof PredAndSucc) {
      adjacentNodeValues.put((N) node, ((PredAndSucc) previousValue).successorValue);
      checkNonNegative(--predecessorCount);
    }
  }

  @SuppressWarnings("unchecked")
  @Override
  public V removeSuccessor(Object node) {
    Object previousValue = adjacentNodeValues.get(node);
    if (previousValue == null || previousValue == PRED) {
      return null;
    } else if (previousValue instanceof PredAndSucc) {
      adjacentNodeValues.put((N) node, PRED);
      checkNonNegative(--successorCount);
      return (V) ((PredAndSucc) previousValue).successorValue;
    } else {       adjacentNodeValues.remove(node);
      checkNonNegative(--successorCount);
      return (V) previousValue;
    }
  }

  @Override
  public void addPredecessor(N node, V unused) {
    Object previousValue = adjacentNodeValues.put(node, PRED);
    if (previousValue == null) {
      checkPositive(++predecessorCount);
    } else if (previousValue instanceof PredAndSucc) {
            adjacentNodeValues.put(node, previousValue);
    } else if (previousValue != PRED) {             adjacentNodeValues.put(node, new PredAndSucc(previousValue));
      checkPositive(++predecessorCount);
    }
  }

  @SuppressWarnings("unchecked")
  @Override
  public V addSuccessor(N node, V value) {
    Object previousValue = adjacentNodeValues.put(node, value);
    if (previousValue == null) {
      checkPositive(++successorCount);
      return null;
    } else if (previousValue instanceof PredAndSucc) {
      adjacentNodeValues.put(node, new PredAndSucc(value));
      return (V) ((PredAndSucc) previousValue).successorValue;
    } else if (previousValue == PRED) {
      adjacentNodeValues.put(node, new PredAndSucc(value));
      checkPositive(++successorCount);
      return null;
    } else {       return (V) previousValue;
    }
  }

  private static boolean isPredecessor(@NullableDecl Object value) {
    return (value == PRED) || (value instanceof PredAndSucc);
  }

  private static boolean isSuccessor(@NullableDecl Object value) {
    return (value != PRED) && (value != null);
  }
}
