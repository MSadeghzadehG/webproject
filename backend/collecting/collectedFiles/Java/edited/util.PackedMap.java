
package hudson.util;

import com.thoughtworks.xstream.converters.UnmarshallingContext;
import com.thoughtworks.xstream.converters.collections.MapConverter;
import com.thoughtworks.xstream.io.HierarchicalStreamReader;
import com.thoughtworks.xstream.mapper.Mapper;

import java.util.AbstractList;
import java.util.AbstractMap;
import java.util.AbstractSet;
import java.util.Collection;
import java.util.HashMap;
import java.util.Iterator;
import java.util.LinkedHashMap;
import java.util.Map;
import java.util.Set;
import java.util.TreeMap;


@SuppressWarnings({"unchecked"})
public final class PackedMap<K,V> extends AbstractMap<K,V> {
    private Object[] kvpairs;

    
    public static <K,V> PackedMap<K,V> of(Map<? extends K,? extends V> src) {
        return new PackedMap<K, V>(src);
    }

    private PackedMap(Map<? extends K,? extends V> src) {
        kvpairs = new Object[src.size()*2];
        int i=0;
        for (Entry<? extends K, ? extends V> e : src.entrySet()) {
            kvpairs[i++] = e.getKey();
            kvpairs[i++] = e.getValue();
        }
    }

    private final Set<Entry<K,V>> entrySet = new AbstractSet<Entry<K, V>>() {
        @Override
        public Iterator<Entry<K, V>> iterator() {
            return new Iterator<Entry<K, V>>() {
                int index=0;
                public boolean hasNext() {
                    return index<kvpairs.length;
                }

                @SuppressWarnings({"unchecked"})
                public Entry<K, V> next() {
                    final K k = (K)kvpairs[index++];
                    final V v = (V)kvpairs[index++];
                    return new Entry<K, V>() {
                        public K getKey() {
                            return k;
                        }

                        public V getValue() {
                            return v;
                        }

                        public V setValue(V value) {
                            throw new UnsupportedOperationException();
                        }
                    };
                }

                public void remove() {
                    throw new UnsupportedOperationException();
                }
            };
        }

        @Override
        public int size() {
            return kvpairs.length/2;
        }
    };

    @Override
    public Set<Entry<K, V>> entrySet() {
        return entrySet;
    }

    @Override
    public boolean containsKey(Object key) {
        for (int i=0; i<kvpairs.length; i+=2)
            if (key.equals(kvpairs[i]))
                return true;
        return false;
    }

    @Override
    public V get(Object key) {
        for (int i=0; i<kvpairs.length; i+=2)
            if (key.equals(kvpairs[i]))
                return (V)kvpairs[i+1];
        return null;
    }

    @Override
    public Collection<V> values() {
        return new AbstractList<V>() {
            @Override
            public V get(int index) {
                return (V)kvpairs[index*2];
            }

            @Override
            public int size() {
                return PackedMap.this.size();
            }
        };
    }

    
    public static class ConverterImpl extends MapConverter {
        public ConverterImpl(Mapper mapper) {
            super(mapper);
        }

        @Override
        public boolean canConvert(Class type) {
            return type==PackedMap.class;
        }

        @Override
        protected Object createCollection(Class type) {
            return new LinkedHashMap<String,String>();
        }

        @Override
        public Object unmarshal(HierarchicalStreamReader reader, UnmarshallingContext context) {
            return PackedMap.of((Map)super.unmarshal(reader, context));
        }
    }
}
