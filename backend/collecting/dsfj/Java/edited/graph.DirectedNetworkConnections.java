

package com.google.common.graph;

import static com.google.common.graph.GraphConstants.EXPECTED_DEGREE;

import com.google.common.collect.BiMap;
import com.google.common.collect.HashBiMap;
import com.google.common.collect.ImmutableBiMap;
import java.util.Collections;
import java.util.Map;
import java.util.Set;


final class DirectedNetworkConnections<N, E> extends AbstractDirectedNetworkConnections<N, E> {

  protected DirectedNetworkConnections(
      Map<E, N> inEdgeMap, Map<E, N> outEdgeMap, int selfLoopCount) {
    super(inEdgeMap, outEdgeMap, selfLoopCount);
  }

  static <N, E> DirectedNetworkConnections<N, E> of() {
    return new DirectedNetworkConnections<>(
        HashBiMap.<E, N>create(EXPECTED_DEGREE), HashBiMap.<E, N>create(EXPECTED_DEGREE), 0);
  }

  static <N, E> DirectedNetworkConnections<N, E> ofImmutable(
      Map<E, N> inEdges, Map<E, N> outEdges, int selfLoopCount) {
    return new DirectedNetworkConnections<>(
        ImmutableBiMap.copyOf(inEdges), ImmutableBiMap.copyOf(outEdges), selfLoopCount);
  }

  @Override
  public Set<N> predecessors() {
    return Collections.unmodifiableSet(((BiMap<E, N>) inEdgeMap).values());
  }

  @Override
  public Set<N> successors() {
    return Collections.unmodifiableSet(((BiMap<E, N>) outEdgeMap).values());
  }

  @Override
  public Set<E> edgesConnecting(N node) {
    return new EdgesConnecting<E>(((BiMap<E, N>) outEdgeMap).inverse(), node);
  }
}
