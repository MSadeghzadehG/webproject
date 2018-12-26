

package com.google.common.graph;

import static com.google.common.base.Preconditions.checkNotNull;
import static com.google.common.graph.GraphConstants.INNER_CAPACITY;
import static com.google.common.graph.GraphConstants.INNER_LOAD_FACTOR;

import com.google.common.collect.ImmutableMap;
import java.util.Collections;
import java.util.HashMap;
import java.util.Map;
import java.util.Set;


final class UndirectedGraphConnections<N, V> implements GraphConnections<N, V> {
  private final Map<N, V> adjacentNodeValues;

  private UndirectedGraphConnections(Map<N, V> adjacentNodeValues) {
    this.adjacentNodeValues = checkNotNull(adjacentNodeValues);
  }

  static <N, V> UndirectedGraphConnections<N, V> of() {
    return new UndirectedGraphConnections<>(new HashMap<N, V>(INNER_CAPACITY, INNER_LOAD_FACTOR));
  }

  static <N, V> UndirectedGraphConnections<N, V> ofImmutable(Map<N, V> adjacentNodeValues) {
    return new UndirectedGraphConnections<>(ImmutableMap.copyOf(adjacentNodeValues));
  }

  @Override
  public Set<N> adjacentNodes() {
    return Collections.unmodifiableSet(adjacentNodeValues.keySet());
  }

  @Override
  public Set<N> predecessors() {
    return adjacentNodes();
  }

  @Override
  public Set<N> successors() {
    return adjacentNodes();
  }

  @Override
  public V value(N node) {
    return adjacentNodeValues.get(node);
  }

  @Override
  public void removePredecessor(N node) {
    @SuppressWarnings("unused")
    V unused = removeSuccessor(node);
  }

  @Override
  public V removeSuccessor(N node) {
    return adjacentNodeValues.remove(node);
  }

  @Override
  public void addPredecessor(N node, V value) {
    @SuppressWarnings("unused")
    V unused = addSuccessor(node, value);
  }

  @Override
  public V addSuccessor(N node, V value) {
    return adjacentNodeValues.put(node, value);
  }
}
