

package com.google.common.collect;

import static com.google.common.base.Preconditions.checkArgument;
import static com.google.common.base.Preconditions.checkState;
import static com.google.common.collect.CollectPreconditions.checkRemove;

import com.google.common.annotations.GwtCompatible;
import com.google.common.annotations.GwtIncompatible;
import com.google.common.base.Objects;
import com.google.errorprone.annotations.CanIgnoreReturnValue;
import com.google.j2objc.annotations.RetainedWith;
import com.google.j2objc.annotations.WeakOuter;
import java.io.IOException;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.io.Serializable;
import java.util.Collection;
import java.util.Iterator;
import java.util.Map;
import java.util.Set;
import java.util.function.BiFunction;
import org.checkerframework.checker.nullness.compatqual.MonotonicNonNullDecl;
import org.checkerframework.checker.nullness.compatqual.NullableDecl;


@GwtCompatible(emulated = true)
abstract class AbstractBiMap<K, V> extends ForwardingMap<K, V>
    implements BiMap<K, V>, Serializable {

  @MonotonicNonNullDecl private transient Map<K, V> delegate;
  @MonotonicNonNullDecl @RetainedWith transient AbstractBiMap<V, K> inverse;

  
  AbstractBiMap(Map<K, V> forward, Map<V, K> backward) {
    setDelegates(forward, backward);
  }

  
  private AbstractBiMap(Map<K, V> backward, AbstractBiMap<V, K> forward) {
    delegate = backward;
    inverse = forward;
  }

  @Override
  protected Map<K, V> delegate() {
    return delegate;
  }

  
  @CanIgnoreReturnValue
  K checkKey(@NullableDecl K key) {
    return key;
  }

  
  @CanIgnoreReturnValue
  V checkValue(@NullableDecl V value) {
    return value;
  }

  
  void setDelegates(Map<K, V> forward, Map<V, K> backward) {
    checkState(delegate == null);
    checkState(inverse == null);
    checkArgument(forward.isEmpty());
    checkArgument(backward.isEmpty());
    checkArgument(forward != backward);
    delegate = forward;
    inverse = makeInverse(backward);
  }

  AbstractBiMap<V, K> makeInverse(Map<V, K> backward) {
    return new Inverse<>(backward, this);
  }

  void setInverse(AbstractBiMap<V, K> inverse) {
    this.inverse = inverse;
  }

  
  @Override
  public boolean containsValue(@NullableDecl Object value) {
    return inverse.containsKey(value);
  }

  
  @CanIgnoreReturnValue
  @Override
  public V put(@NullableDecl K key, @NullableDecl V value) {
    return putInBothMaps(key, value, false);
  }

  @CanIgnoreReturnValue
  @Override
  public V forcePut(@NullableDecl K key, @NullableDecl V value) {
    return putInBothMaps(key, value, true);
  }

  private V putInBothMaps(@NullableDecl K key, @NullableDecl V value, boolean force) {
    checkKey(key);
    checkValue(value);
    boolean containedKey = containsKey(key);
    if (containedKey && Objects.equal(value, get(key))) {
      return value;
    }
    if (force) {
      inverse().remove(value);
    } else {
      checkArgument(!containsValue(value), "value already present: %s", value);
    }
    V oldValue = delegate.put(key, value);
    updateInverseMap(key, containedKey, oldValue, value);
    return oldValue;
  }

  private void updateInverseMap(K key, boolean containedKey, V oldValue, V newValue) {
    if (containedKey) {
      removeFromInverseMap(oldValue);
    }
    inverse.delegate.put(newValue, key);
  }

  @CanIgnoreReturnValue
  @Override
  public V remove(@NullableDecl Object key) {
    return containsKey(key) ? removeFromBothMaps(key) : null;
  }

  @CanIgnoreReturnValue
  private V removeFromBothMaps(Object key) {
    V oldValue = delegate.remove(key);
    removeFromInverseMap(oldValue);
    return oldValue;
  }

  private void removeFromInverseMap(V oldValue) {
    inverse.delegate.remove(oldValue);
  }

  
  @Override
  public void putAll(Map<? extends K, ? extends V> map) {
    for (Entry<? extends K, ? extends V> entry : map.entrySet()) {
      put(entry.getKey(), entry.getValue());
    }
  }

  @Override
  public void replaceAll(BiFunction<? super K, ? super V, ? extends V> function) {
    this.delegate.replaceAll(function);
    inverse.delegate.clear();
    Entry<K, V> broken = null;
    Iterator<Entry<K, V>> itr = this.delegate.entrySet().iterator();
    while (itr.hasNext()) {
      Entry<K, V> entry = itr.next();
      K k = entry.getKey();
      V v = entry.getValue();
      K conflict = inverse.delegate.putIfAbsent(v, k);
      if (conflict != null) {
        broken = entry;
                        itr.remove();
      }
    }
    if (broken != null) {
      throw new IllegalArgumentException("value already present: " + broken.getValue());
    }
  }

  @Override
  public void clear() {
    delegate.clear();
    inverse.delegate.clear();
  }

  
  @Override
  public BiMap<V, K> inverse() {
    return inverse;
  }

  @MonotonicNonNullDecl private transient Set<K> keySet;

  @Override
  public Set<K> keySet() {
    Set<K> result = keySet;
    return (result == null) ? keySet = new KeySet() : result;
  }

  @WeakOuter
  private class KeySet extends ForwardingSet<K> {
    @Override
    protected Set<K> delegate() {
      return delegate.keySet();
    }

    @Override
    public void clear() {
      AbstractBiMap.this.clear();
    }

    @Override
    public boolean remove(Object key) {
      if (!contains(key)) {
        return false;
      }
      removeFromBothMaps(key);
      return true;
    }

    @Override
    public boolean removeAll(Collection<?> keysToRemove) {
      return standardRemoveAll(keysToRemove);
    }

    @Override
    public boolean retainAll(Collection<?> keysToRetain) {
      return standardRetainAll(keysToRetain);
    }

    @Override
    public Iterator<K> iterator() {
      return Maps.keyIterator(entrySet().iterator());
    }
  }

  @MonotonicNonNullDecl private transient Set<V> valueSet;

  @Override
  public Set<V> values() {
    
    Set<V> result = valueSet;
    return (result == null) ? valueSet = new ValueSet() : result;
  }

  @WeakOuter
  private class ValueSet extends ForwardingSet<V> {
    final Set<V> valuesDelegate = inverse.keySet();

    @Override
    protected Set<V> delegate() {
      return valuesDelegate;
    }

    @Override
    public Iterator<V> iterator() {
      return Maps.valueIterator(entrySet().iterator());
    }

    @Override
    public Object[] toArray() {
      return standardToArray();
    }

    @Override
    public <T> T[] toArray(T[] array) {
      return standardToArray(array);
    }

    @Override
    public String toString() {
      return standardToString();
    }
  }

  @MonotonicNonNullDecl private transient Set<Entry<K, V>> entrySet;

  @Override
  public Set<Entry<K, V>> entrySet() {
    Set<Entry<K, V>> result = entrySet;
    return (result == null) ? entrySet = new EntrySet() : result;
  }

  class BiMapEntry extends ForwardingMapEntry<K, V> {
    private final Entry<K, V> delegate;

    BiMapEntry(Entry<K, V> delegate) {
      this.delegate = delegate;
    }

    @Override
    protected Entry<K, V> delegate() {
      return delegate;
    }

    @Override
    public V setValue(V value) {
      checkValue(value);
            checkState(entrySet().contains(this), "entry no longer in map");
            if (Objects.equal(value, getValue())) {
        return value;
      }
      checkArgument(!containsValue(value), "value already present: %s", value);
      V oldValue = delegate.setValue(value);
      checkState(Objects.equal(value, get(getKey())), "entry no longer in map");
      updateInverseMap(getKey(), true, oldValue, value);
      return oldValue;
    }
  }

  Iterator<Entry<K, V>> entrySetIterator() {
    final Iterator<Entry<K, V>> iterator = delegate.entrySet().iterator();
    return new Iterator<Entry<K, V>>() {
      @NullableDecl Entry<K, V> entry;

      @Override
      public boolean hasNext() {
        return iterator.hasNext();
      }

      @Override
      public Entry<K, V> next() {
        entry = iterator.next();
        return new BiMapEntry(entry);
      }

      @Override
      public void remove() {
        checkRemove(entry != null);
        V value = entry.getValue();
        iterator.remove();
        removeFromInverseMap(value);
        entry = null;
      }
    };
  }

  @WeakOuter
  private class EntrySet extends ForwardingSet<Entry<K, V>> {
    final Set<Entry<K, V>> esDelegate = delegate.entrySet();

    @Override
    protected Set<Entry<K, V>> delegate() {
      return esDelegate;
    }

    @Override
    public void clear() {
      AbstractBiMap.this.clear();
    }

    @Override
    public boolean remove(Object object) {
      if (!esDelegate.contains(object)) {
        return false;
      }

            Entry<?, ?> entry = (Entry<?, ?>) object;
      inverse.delegate.remove(entry.getValue());
      
      esDelegate.remove(entry);
      return true;
    }

    @Override
    public Iterator<Entry<K, V>> iterator() {
      return entrySetIterator();
    }

    
    @Override
    public Object[] toArray() {
      return standardToArray();
    }

    @Override
    public <T> T[] toArray(T[] array) {
      return standardToArray(array);
    }

    @Override
    public boolean contains(Object o) {
      return Maps.containsEntryImpl(delegate(), o);
    }

    @Override
    public boolean containsAll(Collection<?> c) {
      return standardContainsAll(c);
    }

    @Override
    public boolean removeAll(Collection<?> c) {
      return standardRemoveAll(c);
    }

    @Override
    public boolean retainAll(Collection<?> c) {
      return standardRetainAll(c);
    }
  }

  
  static class Inverse<K, V> extends AbstractBiMap<K, V> {
    Inverse(Map<K, V> backward, AbstractBiMap<V, K> forward) {
      super(backward, forward);
    }

    

    @Override
    K checkKey(K key) {
      return inverse.checkValue(key);
    }

    @Override
    V checkValue(V value) {
      return inverse.checkKey(value);
    }

    
    @GwtIncompatible     private void writeObject(ObjectOutputStream stream) throws IOException {
      stream.defaultWriteObject();
      stream.writeObject(inverse());
    }

    @GwtIncompatible     @SuppressWarnings("unchecked")     private void readObject(ObjectInputStream stream) throws IOException, ClassNotFoundException {
      stream.defaultReadObject();
      setInverse((AbstractBiMap<V, K>) stream.readObject());
    }

    @GwtIncompatible     Object readResolve() {
      return inverse().inverse();
    }

    @GwtIncompatible     private static final long serialVersionUID = 0;
  }

  @GwtIncompatible   private static final long serialVersionUID = 0;
}
