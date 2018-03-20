package hudson.util;

import hudson.Util;

import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import java.util.Set;
import java.util.Stack;


public abstract class CyclicGraphDetector<N> {
    private final Set<N> visited = new HashSet<N>();
    private final Set<N> visiting = new HashSet<N>();
    private final Stack<N> path = new Stack<N>();

    private final List<N> topologicalOrder = new ArrayList<N>();

    public void run(Iterable<? extends N> allNodes) throws CycleDetectedException {
        for (N n : allNodes){
            visit(n);
        }
    }

    
    public List<N> getSorted() {
        return topologicalOrder;
    }

    
    protected abstract Iterable<? extends N> getEdges(N n);

    private void visit(N p) throws CycleDetectedException {
        if (!visited.add(p))    return;

        visiting.add(p);
        path.push(p);
        for (N q : getEdges(p)) {
            if (q==null)        continue;               if (visiting.contains(q))
                detectedCycle(q);
            visit(q);
        }
        visiting.remove(p);
        path.pop();
        topologicalOrder.add(p);
    }

    private void detectedCycle(N q) throws CycleDetectedException {
        int i = path.indexOf(q);
        path.push(q);
        reactOnCycle(q, path.subList(i, path.size()));
    }
    
    
    protected void reactOnCycle(N q, List<N> cycle) throws CycleDetectedException{
        throw new CycleDetectedException(cycle);
    }    

    public static final class CycleDetectedException extends Exception {
        public final List cycle;

        public CycleDetectedException(List cycle) {
            super("Cycle detected: "+Util.join(cycle," -> "));
            this.cycle = cycle;
        }
    }
}
