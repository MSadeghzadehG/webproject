

package org.elasticsearch.painless.node;

import org.elasticsearch.painless.Globals;
import org.elasticsearch.painless.Locals;
import org.elasticsearch.painless.Location;
import org.elasticsearch.painless.MethodWriter;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;
import java.util.Iterator;
import java.util.List;
import java.util.Objects;
import java.util.Set;

import static java.lang.Math.max;
import static java.util.Collections.emptyList;


public abstract class ANode {

    
    final Location location;

    
    ANode(Location location) {
        this.location = Objects.requireNonNull(location);
    }

    
    abstract void extractVariables(Set<String> variables);

    
    abstract void analyze(Locals locals);

    
    abstract void write(MethodWriter writer, Globals globals);

    
    RuntimeException createError(RuntimeException exception) {
        return location.createError(exception);
    }

    
    public abstract String toString();

    
    
    protected String singleLineToString(Object... subs) {
        return singleLineToString(Arrays.asList(subs));
    }

    
    protected String singleLineToString(Collection<? extends Object> subs) {
        return joinWithName(getClass().getSimpleName(), subs, emptyList());
    }

    
    protected String singleLineToStringWithOptionalArgs(Collection<? extends ANode> arguments, Object... restOfSubs) {
        List<Object> subs = new ArrayList<>();
        Collections.addAll(subs, restOfSubs);
        if (false == arguments.isEmpty()) {
            subs.add(joinWithName("Args", arguments, emptyList()));
        }
        return singleLineToString(subs);
    }

    
    protected String multilineToString(Collection<? extends Object> sameLine, Collection<? extends Object> ownLine) {
        return joinWithName(getClass().getSimpleName(), sameLine, ownLine);
    }

    
    protected List<String> pairwiseToString(Collection<? extends Object> lefts, Collection<? extends Object> rights) {
        List<String> pairs = new ArrayList<>(max(lefts.size(), rights.size()));
        Iterator<? extends Object> left = lefts.iterator();
        Iterator<? extends Object> right = rights.iterator();
        while (left.hasNext() || right.hasNext()) {
            pairs.add(joinWithName("Pair",
                    Arrays.asList(left.hasNext() ? left.next() : "<uneven>", right.hasNext() ? right.next() : "<uneven>"),
                    emptyList()));
        }
        return pairs;
    }

    
    protected String joinWithName(String name, Collection<? extends Object> sameLine,
            Collection<? extends Object> ownLine) {
        StringBuilder b = new StringBuilder();
        b.append('(').append(name);
        for (Object sub : sameLine) {
            b.append(' ').append(sub);
        }
        if (ownLine.size() == 1 && sameLine.isEmpty()) {
            b.append(' ').append(ownLine.iterator().next());
        } else {
            for (Object sub : ownLine) {
                b.append("\n  ").append(Objects.toString(sub).replace("\n", "\n  "));
            }
        }
        return b.append(')').toString();
    }
}
