package jenkins.util;

import java.util.AbstractSet;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.Stack;


public abstract class DirectedGraph<N> {
    
    protected abstract Collection<N> nodes();

    
    protected abstract Collection<N> forward(N node);

    
    public static class SCC<N> extends AbstractSet<N> {
        
        public final int index;

        private final List<N> members = new ArrayList<N>();

        public SCC(int index) {
            this.index = index;
        }

        @Override
        public Iterator<N> iterator() {
            return members.iterator();
        }

        @Override
        public int size() {
            return members.size();
        }
    }

    
    class Node {
        final N n;
        
        int index = -1;
        
        int lowlink;

        SCC scc;

        Node(N n) {
            this.n = n;
        }

        Collection<N> edges() {
            return forward(n);
        }
    }

    
    public List<SCC<N>> getStronglyConnectedComponents() {
        final Map<N, Node> nodes = new HashMap<N, Node>();
        for (N n : nodes()) {
            nodes.put(n,new Node(n));
        }

        final List<SCC<N>> sccs = new ArrayList<SCC<N>>();

        class Tarjan {
            int index = 0;
            int sccIndex = 0;
            
            Stack<Node> pending = new Stack<Node>();
            
            void traverse() {
                for (Node n : nodes.values()) {
                    if (n.index==-1)
                        visit(n);
                }
            }
            
            void visit(Node v) {
                v.index = v.lowlink = index++;
                pending.push(v);

                for (N q : v.edges()) {
                    Node w = nodes.get(q);
                    if (w.index==-1) {
                        visit(w);
                        v.lowlink = Math.min(v.lowlink,w.lowlink);
                    } else
                    if (pending.contains(w)) {
                        v.lowlink = Math.min(v.lowlink,w.index);
                    }
                }

                if (v.lowlink==v.index) {
                                        SCC<N> scc = new SCC<N>(sccIndex++);
                    sccs.add(scc);

                    Node w;
                    do {
                        w = pending.pop();
                        w.scc = scc;
                        scc.members.add(w.n);
                    } while(w!=v);
                }
            }
        }

        new Tarjan().traverse();

        Collections.reverse(sccs);

        return sccs;
    }
}
