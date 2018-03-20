

package org.elasticsearch.common.collect;

import java.util.HashMap;
import java.util.Map;

import static java.util.Collections.unmodifiableMap;

public class MapBuilder<K, V> {

    public static <K, V> MapBuilder<K, V> newMapBuilder() {
        return new MapBuilder<>();
    }

    public static <K, V> MapBuilder<K, V> newMapBuilder(Map<K, V> map) {
        return new MapBuilder<>(map);
    }

    private Map<K, V> map = new HashMap<>();

    public MapBuilder() {
        this.map = new HashMap<>();
    }

    public MapBuilder(Map<K, V> map) {
        this.map = new HashMap<>(map);
    }

    public MapBuilder<K, V> putAll(Map<K, V> map) {
        this.map.putAll(map);
        return this;
    }

    public MapBuilder<K, V> put(K key, V value) {
        this.map.put(key, value);
        return this;
    }

    public MapBuilder<K, V> remove(K key) {
        this.map.remove(key);
        return this;
    }

    public MapBuilder<K, V> clear() {
        this.map.clear();
        return this;
    }

    public V get(K key) {
        return map.get(key);
    }

    public boolean containsKey(K key) {
        return map.containsKey(key);
    }

    public boolean isEmpty() {
        return map.isEmpty();
    }

    public Map<K, V> map() {
        return this.map;
    }

    
    public Map<K, V> immutableMap() {
                return unmodifiableMap(new HashMap<>(map));
    }
}
