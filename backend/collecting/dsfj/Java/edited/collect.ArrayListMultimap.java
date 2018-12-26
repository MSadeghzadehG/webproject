

package com.google.common.collect;

import static com.google.common.collect.CollectPreconditions.checkNonnegative;

import com.google.common.annotations.GwtCompatible;
import com.google.common.annotations.GwtIncompatible;
import com.google.common.annotations.VisibleForTesting;
import java.io.IOException;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.util.ArrayList;
import java.util.Collection;
import java.util.HashMap;
import java.util.List;
import java.util.Map;


@GwtCompatible(serializable = true, emulated = true)
public final class ArrayListMultimap<K, V>
    extends ArrayListMultimapGwtSerializationDependencies<K, V> {
    private static final int DEFAULT_VALUES_PER_KEY = 3;

  @VisibleForTesting transient int expectedValuesPerKey;

  
  public static <K, V> ArrayListMultimap<K, V> create() {
    return new ArrayListMultimap<>();
  }

  
  public static <K, V> ArrayListMultimap<K, V> create(int expectedKeys, int expectedValuesPerKey) {
    return new ArrayListMultimap<>(expectedKeys, expectedValuesPerKey);
  }

  
  public static <K, V> ArrayListMultimap<K, V> create(Multimap<? extends K, ? extends V> multimap) {
    return new ArrayListMultimap<>(multimap);
  }

  private ArrayListMultimap() {
    this(12, DEFAULT_VALUES_PER_KEY);
  }

  private ArrayListMultimap(int expectedKeys, int expectedValuesPerKey) {
    super(Platform.<K, Collection<V>>newHashMapWithExpectedSize(expectedKeys));
    checkNonnegative(expectedValuesPerKey, "expectedValuesPerKey");
    this.expectedValuesPerKey = expectedValuesPerKey;
  }

  private ArrayListMultimap(Multimap<? extends K, ? extends V> multimap) {
    this(
        multimap.keySet().size(),
        (multimap instanceof ArrayListMultimap)
            ? ((ArrayListMultimap<?, ?>) multimap).expectedValuesPerKey
            : DEFAULT_VALUES_PER_KEY);
    putAll(multimap);
  }

  
  @Override
  List<V> createCollection() {
    return new ArrayList<V>(expectedValuesPerKey);
  }

  
  @Deprecated
  public void trimToSize() {
    for (Collection<V> collection : backingMap().values()) {
      ArrayList<V> arrayList = (ArrayList<V>) collection;
      arrayList.trimToSize();
    }
  }

  
  @GwtIncompatible   private void writeObject(ObjectOutputStream stream) throws IOException {
    stream.defaultWriteObject();
    Serialization.writeMultimap(this, stream);
  }

  @GwtIncompatible   private void readObject(ObjectInputStream stream) throws IOException, ClassNotFoundException {
    stream.defaultReadObject();
    expectedValuesPerKey = DEFAULT_VALUES_PER_KEY;
    int distinctKeys = Serialization.readCount(stream);
    Map<K, Collection<V>> map = CompactHashMap.create();
    setMap(map);
    Serialization.populateMultimap(this, stream, distinctKeys);
  }

  @GwtIncompatible   private static final long serialVersionUID = 0;
}
