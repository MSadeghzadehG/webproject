

package org.elasticsearch.common.inject;

import java.util.HashSet;
import java.util.Set;


final class WeakKeySet {

    
    private Set<String> backingSet = new HashSet<>();

    public boolean add(Key<?> key) {
        return backingSet.add(key.toString());
    }

    public boolean contains(Object o) {
        return o instanceof Key && backingSet.contains(o.toString());
    }
}
