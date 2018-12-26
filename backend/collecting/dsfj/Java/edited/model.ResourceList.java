
package hudson.model;

import java.util.Set;
import java.util.HashSet;
import java.util.Map;
import java.util.HashMap;
import java.util.Arrays;
import java.util.Collection;
import java.util.Map.Entry;
import java.util.logging.Logger;


public final class ResourceList {

    private static final Logger LOGGER = Logger.getLogger(ResourceList.class.getName());

    
    private final Set<Resource> all = new HashSet<Resource>();

    
    private final Map<Resource,Integer> write = new HashMap<Resource,Integer>();
    private static final Integer MAX_INT = Integer.MAX_VALUE;

    
    public static ResourceList union(ResourceList... lists) {
        return union(Arrays.asList(lists));
    }

    
    public static ResourceList union(Collection<ResourceList> lists) {
        switch(lists.size()) {
        case 0:
            return EMPTY;
        case 1:
            return lists.iterator().next();
        default:
            ResourceList r = new ResourceList();
            for (ResourceList l : lists) {
                r.all.addAll(l.all);
                for (Entry<Resource, Integer> e : l.write.entrySet())
                    r.write.put(e.getKey(), unbox(r.write.get(e.getKey()))+e.getValue());
            }
            return r;
        }
    }

    
    public ResourceList r(Resource r) {
        all.add(r);
        return this;
    }

    
    public ResourceList w(Resource r) {
        all.add(r);
        write.put(r, unbox(write.get(r))+1);
        return this;
    }

    
    public boolean isCollidingWith(ResourceList that) {
        return getConflict(that)!=null;
    }

    
    public Resource getConflict(ResourceList that) {
        Resource r = _getConflict(this,that);
        if(r!=null)     return r;
        return _getConflict(that,this);
    }

    private Resource _getConflict(ResourceList lhs, ResourceList rhs) {
        for (Entry<Resource,Integer> r : lhs.write.entrySet()) {
            for (Resource l : rhs.all) {
                Integer v = rhs.write.get(l);
                if(v!=null)                     v += r.getValue();
                else                     v = MAX_INT;
                if(r.getKey().isCollidingWith(l,unbox(v))) {
                    LOGGER.info("Collision with " + r + " and " + l);
                    return r.getKey();
                }
            }
        }
        return null;
    }

    @Override
    public String toString() {
        Map<Resource,String> m = new HashMap<Resource,String>();
        for (Resource r : all)
            m.put(r,"R");
        for (Entry<Resource,Integer> e : write.entrySet())
            m.put(e.getKey(),"W"+e.getValue());
        return m.toString();
    }

    
    private static int unbox(Integer x) {
        return x==null ? 0 : x;
    }

    
    public static final ResourceList EMPTY = new ResourceList();
}
