

package com.google.common.collect;

import com.google.common.annotations.GwtCompatible;
import com.google.common.annotations.GwtIncompatible;
import com.google.j2objc.annotations.Weak;
import java.io.Serializable;
import java.util.Map.Entry;
import java.util.Spliterator;
import java.util.function.Consumer;
import org.checkerframework.checker.nullness.compatqual.NullableDecl;


@GwtCompatible(emulated = true)
abstract class ImmutableMapEntrySet<K, V> extends ImmutableSet<Entry<K, V>> {
  static final class RegularEntrySet<K, V> extends ImmutableMapEntrySet<K, V> {
    @Weak private final transient ImmutableMap<K, V> map;
    private final transient ImmutableList<Entry<K, V>> entries;

    RegularEntrySet(ImmutableMap<K, V> map, Entry<K, V>[] entries) {
      this(map, ImmutableList.<Entry<K, V>>asImmutableList(entries));
    }

    RegularEntrySet(ImmutableMap<K, V> map, ImmutableList<Entry<K, V>> entries) {
      this.map = map;
      this.entries = entries;
    }

    @Override
    ImmutableMap<K, V> map() {
      return map;
    }

    @Override
    @GwtIncompatible("not used in GWT")
    int copyIntoArray(Object[] dst, int offset) {
      return entries.copyIntoArray(dst, offset);
    }

    @Override
    public UnmodifiableIterator<Entry<K, V>> iterator() {
      return entries.iterator();
    }

    @Override
    public Spliterator<Entry<K, V>> spliterator() {
      return entries.spliterator();
    }

    @Override
    public void forEach(Consumer<? super Entry<K, V>> action) {
      entries.forEach(action);
    }

    @Override
    ImmutableList<Entry<K, V>> createAsList() {
      return new RegularImmutableAsList<>(this, entries);
    }
  }

  ImmutableMapEntrySet() {}

  abstract ImmutableMap<K, V> map();

  @Override
  public int size() {
    return map().size();
  }

  @Override
  public boolean contains(@NullableDecl Object object) {
    if (object instanceof Entry) {
      Entry<?, ?> entry = (Entry<?, ?>) object;
      V value = map().get(entry.getKey());
      return value != null && value.equals(entry.getValue());
    }
    return false;
  }

  @Override
  boolean isPartialView() {
    return map().isPartialView();
  }

  @Override
  @GwtIncompatible   boolean isHashCodeFast() {
    return map().isHashCodeFast();
  }

  @Override
  public int hashCode() {
    return map().hashCode();
  }

  @GwtIncompatible   @Override
  Object writeReplace() {
    return new EntrySetSerializedForm<>(map());
  }

  @GwtIncompatible   private static class EntrySetSerializedForm<K, V> implements Serializable {
    final ImmutableMap<K, V> map;

    EntrySetSerializedForm(ImmutableMap<K, V> map) {
      this.map = map;
    }

    Object readResolve() {
      return map.entrySet();
    }

    private static final long serialVersionUID = 0;
  }
}
