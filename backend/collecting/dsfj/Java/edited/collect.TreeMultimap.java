

package com.google.common.collect;

import static com.google.common.base.Preconditions.checkNotNull;

import com.google.common.annotations.GwtCompatible;
import com.google.common.annotations.GwtIncompatible;
import java.io.IOException;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.util.Collection;
import java.util.Comparator;
import java.util.Map;
import java.util.NavigableMap;
import java.util.NavigableSet;
import java.util.SortedSet;
import java.util.TreeMap;
import java.util.TreeSet;
import org.checkerframework.checker.nullness.compatqual.NullableDecl;


@GwtCompatible(serializable = true, emulated = true)
public class TreeMultimap<K, V> extends AbstractSortedKeySortedSetMultimap<K, V> {
  private transient Comparator<? super K> keyComparator;
  private transient Comparator<? super V> valueComparator;

  
  public static <K extends Comparable, V extends Comparable> TreeMultimap<K, V> create() {
    return new TreeMultimap<>(Ordering.natural(), Ordering.natural());
  }

  
  public static <K, V> TreeMultimap<K, V> create(
      Comparator<? super K> keyComparator, Comparator<? super V> valueComparator) {
    return new TreeMultimap<>(checkNotNull(keyComparator), checkNotNull(valueComparator));
  }

  
  public static <K extends Comparable, V extends Comparable> TreeMultimap<K, V> create(
      Multimap<? extends K, ? extends V> multimap) {
    return new TreeMultimap<>(Ordering.natural(), Ordering.natural(), multimap);
  }

  TreeMultimap(Comparator<? super K> keyComparator, Comparator<? super V> valueComparator) {
    super(new TreeMap<K, Collection<V>>(keyComparator));
    this.keyComparator = keyComparator;
    this.valueComparator = valueComparator;
  }

  private TreeMultimap(
      Comparator<? super K> keyComparator,
      Comparator<? super V> valueComparator,
      Multimap<? extends K, ? extends V> multimap) {
    this(keyComparator, valueComparator);
    putAll(multimap);
  }

  @Override
  Map<K, Collection<V>> createAsMap() {
    return createMaybeNavigableAsMap();
  }

  
  @Override
  SortedSet<V> createCollection() {
    return new TreeSet<V>(valueComparator);
  }

  @Override
  Collection<V> createCollection(@NullableDecl K key) {
    if (key == null) {
      keyComparator().compare(key, key);
    }
    return super.createCollection(key);
  }

  
  @Deprecated
  public Comparator<? super K> keyComparator() {
    return keyComparator;
  }

  @Override
  public Comparator<? super V> valueComparator() {
    return valueComparator;
  }

  
  @Override
  @GwtIncompatible   public NavigableSet<V> get(@NullableDecl K key) {
    return (NavigableSet<V>) super.get(key);
  }

  
  @Override
  public NavigableSet<K> keySet() {
    return (NavigableSet<K>) super.keySet();
  }

  
  @Override
  public NavigableMap<K, Collection<V>> asMap() {
    return (NavigableMap<K, Collection<V>>) super.asMap();
  }

  
  @GwtIncompatible   private void writeObject(ObjectOutputStream stream) throws IOException {
    stream.defaultWriteObject();
    stream.writeObject(keyComparator());
    stream.writeObject(valueComparator());
    Serialization.writeMultimap(this, stream);
  }

  @GwtIncompatible   @SuppressWarnings("unchecked")   private void readObject(ObjectInputStream stream) throws IOException, ClassNotFoundException {
    stream.defaultReadObject();
    keyComparator = checkNotNull((Comparator<? super K>) stream.readObject());
    valueComparator = checkNotNull((Comparator<? super V>) stream.readObject());
    setMap(new TreeMap<K, Collection<V>>(keyComparator));
    Serialization.populateMultimap(this, stream);
  }

  @GwtIncompatible   private static final long serialVersionUID = 0;
}
