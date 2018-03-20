

package com.google.common.graph;

import static com.google.common.base.Preconditions.checkNotNull;
import static com.google.common.base.Preconditions.checkState;

import java.util.Collections;
import java.util.Map;
import java.util.Set;


abstract class AbstractUndirectedNetworkConnections<N, E> implements NetworkConnections<N, E> {
  
  protected final Map<E, N> incidentEdgeMap;

  protected AbstractUndirectedNetworkConnections(Map<E, N> incidentEdgeMap) {
    this.incidentEdgeMap = checkNotNull(incidentEdgeMap);
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
  public Set<E> incidentEdges() {
    return Collections.unmodifiableSet(incidentEdgeMap.keySet());
  }

  @Override
  public Set<E> inEdges() {
    return incidentEdges();
  }

  @Override
  public Set<E> outEdges() {
    return incidentEdges();
  }

  @Override
  public N adjacentNode(E edge) {
    return checkNotNull(incidentEdgeMap.get(edge));
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
    N previousNode = incidentEdgeMap.remove(edge);
    return checkNotNull(previousNode);
  }

  @Override
  public void addInEdge(E edge, N node, boolean isSelfLoop) {
    if (!isSelfLoop) {
      addOutEdge(edge, node);
    }
  }

  @Override
  public void addOutEdge(E edge, N node) {
    N previousNode = incidentEdgeMap.put(edge, node);
    checkState(previousNode == null);
  }
}
