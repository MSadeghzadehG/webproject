

package com.google.common.graph;

import static com.google.common.truth.Truth.assertThat;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.Random;
import java.util.RandomAccess;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;


@RunWith(JUnit4.class)

public final class NetworkMutationTest {
  private static final int NUM_TRIALS = 25;
  private static final int NUM_NODES = 100;
  private static final int NUM_EDGES = 1000;
  private static final int NODE_POOL_SIZE = 1000; 
  @Test
  public void directedNetwork() {
    testNetworkMutation(NetworkBuilder.directed());
  }

  @Test
  public void undirectedNetwork() {
    testNetworkMutation(NetworkBuilder.undirected());
  }

  private static void testNetworkMutation(NetworkBuilder<? super Integer, Object> networkBuilder) {
    Random gen = new Random(42); 
    for (int trial = 0; trial < NUM_TRIALS; ++trial) {
      MutableNetwork<Integer, Object> network =
          networkBuilder.allowsParallelEdges(true).allowsSelfLoops(true).build();

      assertThat(network.nodes()).isEmpty();
      assertThat(network.edges()).isEmpty();
      AbstractNetworkTest.validateNetwork(network);

      while (network.nodes().size() < NUM_NODES) {
        network.addNode(gen.nextInt(NODE_POOL_SIZE));
      }
      ArrayList<Integer> nodeList = new ArrayList<>(network.nodes());
      for (int i = 0; i < NUM_EDGES; ++i) {
                assertThat(
                network.addEdge(
                    getRandomElement(nodeList, gen), getRandomElement(nodeList, gen), new Object()))
            .isTrue();
      }
      ArrayList<Object> edgeList = new ArrayList<>(network.edges());

      assertThat(network.nodes()).hasSize(NUM_NODES);
      assertThat(network.edges()).hasSize(NUM_EDGES);
      AbstractNetworkTest.validateNetwork(network);

      Collections.shuffle(edgeList, gen);
      int numEdgesToRemove = gen.nextInt(NUM_EDGES);
      for (int i = 0; i < numEdgesToRemove; ++i) {
        Object edge = edgeList.get(i);
        assertThat(network.removeEdge(edge)).isTrue();
      }

      assertThat(network.nodes()).hasSize(NUM_NODES);
      assertThat(network.edges()).hasSize(NUM_EDGES - numEdgesToRemove);
      AbstractNetworkTest.validateNetwork(network);

      Collections.shuffle(nodeList, gen);
      int numNodesToRemove = gen.nextInt(NUM_NODES);
      for (int i = 0; i < numNodesToRemove; ++i) {
        assertThat(network.removeNode(nodeList.get(i))).isTrue();
      }

      assertThat(network.nodes()).hasSize(NUM_NODES - numNodesToRemove);
            AbstractNetworkTest.validateNetwork(network);

      for (int i = numNodesToRemove; i < NUM_NODES; ++i) {
        assertThat(network.removeNode(nodeList.get(i))).isTrue();
      }

      assertThat(network.nodes()).isEmpty();
      assertThat(network.edges()).isEmpty();       AbstractNetworkTest.validateNetwork(network);

      Collections.shuffle(nodeList, gen);
      for (Integer node : nodeList) {
        assertThat(network.addNode(node)).isTrue();
      }
      Collections.shuffle(edgeList, gen);
      for (Object edge : edgeList) {
        assertThat(
                network.addEdge(
                    getRandomElement(nodeList, gen), getRandomElement(nodeList, gen), edge))
            .isTrue();
      }

      assertThat(network.nodes()).hasSize(NUM_NODES);
      assertThat(network.edges()).hasSize(NUM_EDGES);
      AbstractNetworkTest.validateNetwork(network);
    }
  }

  private static <L extends List<T> & RandomAccess, T> T getRandomElement(L list, Random gen) {
    return list.get(gen.nextInt(list.size()));
  }
}
