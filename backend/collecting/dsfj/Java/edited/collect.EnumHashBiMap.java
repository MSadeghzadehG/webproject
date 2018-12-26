

package com.google.common.collect;

import static com.google.common.base.Preconditions.checkNotNull;

import com.google.common.annotations.GwtCompatible;
import com.google.common.annotations.GwtIncompatible;
import com.google.errorprone.annotations.CanIgnoreReturnValue;
import java.io.IOException;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.util.EnumMap;
import java.util.HashMap;
import java.util.Map;
import org.checkerframework.checker.nullness.compatqual.NullableDecl;


@GwtCompatible(emulated = true)
public final class EnumHashBiMap<K extends Enum<K>, V> extends AbstractBiMap<K, V> {
  private transient Class<K> keyType;

  
  public static <K extends Enum<K>, V> EnumHashBiMap<K, V> create(Class<K> keyType) {
    return new EnumHashBiMap<>(keyType);
  }

  
  public static <K extends Enum<K>, V> EnumHashBiMap<K, V> create(Map<K, ? extends V> map) {
    EnumHashBiMap<K, V> bimap = create(EnumBiMap.inferKeyType(map));
    bimap.putAll(map);
    return bimap;
  }

  private EnumHashBiMap(Class<K> keyType) {
    super(
        WellBehavedMap.wrap(new EnumMap<K, V>(keyType)),
        Maps.<V, K>newHashMapWithExpectedSize(keyType.getEnumConstants().length));
    this.keyType = keyType;
  }

  
  @Override
  K checkKey(K key) {
    return checkNotNull(key);
  }

  @CanIgnoreReturnValue
  @Override
  public V put(K key, @NullableDecl V value) {
    return super.put(key, value);
  }

  @CanIgnoreReturnValue
  @Override
  public V forcePut(K key, @NullableDecl V value) {
    return super.forcePut(key, value);
  }

  
  public Class<K> keyType() {
    return keyType;
  }

  
  @GwtIncompatible   private void writeObject(ObjectOutputStream stream) throws IOException {
    stream.defaultWriteObject();
    stream.writeObject(keyType);
    Serialization.writeMap(this, stream);
  }

  @SuppressWarnings("unchecked")   @GwtIncompatible   private void readObject(ObjectInputStream stream) throws IOException, ClassNotFoundException {
    stream.defaultReadObject();
    keyType = (Class<K>) stream.readObject();
    setDelegates(
        WellBehavedMap.wrap(new EnumMap<K, V>(keyType)),
        new HashMap<V, K>(keyType.getEnumConstants().length * 3 / 2));
    Serialization.populateMap(this, stream);
  }

  @GwtIncompatible   private static final long serialVersionUID = 0;
}
