

package com.google.common.graph;

import static com.google.common.base.Preconditions.checkState;
import static com.google.common.graph.GraphConstants.INNER_CAPACITY;
import static com.google.common.graph.GraphConstants.INNER_LOAD_FACTOR;

import com.google.common.collect.HashMultiset;
import com.google.common.collect.ImmutableMap;
import com.google.common.collect.Multiset;
import com.google.errorprone.annotations.concurrent.LazyInit;
import java.lang.ref.Reference;
import java.lang.ref.SoftReference;
import java.util.Collections;
import java.util.HashMap;
import java.util.Map;
import java.util.Set;
import org.checkerframework.checker.nullness.compatqual.NullableDecl;


final class UndirectedMultiNetworkConnections<N, E>
    extends AbstractUndirectedNetworkConnections<N, E> {

  private UndirectedMultiNetworkConnections(Map<E, N> incidentEdges) {
    super(incidentEdges);
  }

  static <N, E> UndirectedMultiNetworkConnections<N, E> of() {
    return new UndirectedMultiNetworkConnections<>(
        new HashMap<E, N>(INNER_CAPACITY, INNER_LOAD_FACTOR));
  }

  static <N, E> UndirectedMultiNetworkConnections<N, E> ofImmutable(Map<E, N> incidentEdges) {
    return new UndirectedMultiNetworkConnections<>(ImmutableMap.copyOf(incidentEdges));
  }

  @LazyInit private transient Reference<Multiset<N>> adjacentNodesReference;

  @Override
  public Set<N> adjacentNodes() {
    return Collections.unmodifiableSet(adjacentNodesMultiset().elementSet());
  }

  private Multiset<N> adjacentNodesMultiset() {
    Multiset<N> adjacentNodes = getReference(adjacentNodesReference);
    if (adjacentNodes == null) {
      adjacentNodes = HashMultiset.create(incidentEdgeMap.values());
      adjacentNodesReference = new SoftReference<>(adjacentNodes);
    }
    return adjacentNodes;
  }

  @Override
  public Set<E> edgesConnecting(final N node) {
    return new MultiEdgesConnecting<E>(incidentEdgeMap, node) {
      @Override
      public int size() {
        return adjacentNodesMultiset().count(node);
      }
    };
  }

  @Override
  public N removeInEdge(E edge, boolean isSelfLoop) {
    if (!isSelfLoop) {
      return removeOutEdge(edge);
    }
    return null;
  }

  @Override
  public N removeOutEdge(E edge) {
    N node = super.removeOutEdge(edge);
    Multiset<N> adjacentNodes = getReference(adjacentNodesReference);
    if (adjacentNodes != null) {
      checkState(adjacentNodes.remove(node));
    }
    return node;
  }

  @Override
  public void addInEdge(E edge, N node, boolean isSelfLoop) {
    if (!isSelfLoop) {
      addOutEdge(edge, node);
    }
  }

  @Override
  public void addOutEdge(E edge, N node) {
    super.addOutEdge(edge, node);
    Multiset<N> adjacentNodes = getReference(adjacentNodesReference);
    if (adjacentNodes != null) {
      checkState(adjacentNodes.add(node));
    }
  }

  @NullableDecl
  private static <T> T getReference(@NullableDecl Reference<T> reference) {
    return (reference == null) ? null : reference.get();
  }
}
