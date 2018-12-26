

package com.google.common.collect;

import static com.google.common.base.Preconditions.checkNotNull;

import com.google.common.annotations.GwtIncompatible;
import com.google.common.primitives.Primitives;
import com.google.errorprone.annotations.CanIgnoreReturnValue;
import java.io.Serializable;
import java.util.HashMap;
import java.util.Iterator;
import java.util.LinkedHashMap;
import java.util.Map;
import java.util.Set;


@GwtIncompatible
@SuppressWarnings("serial") public final class MutableClassToInstanceMap<B> extends ForwardingMap<Class<? extends B>, B>
    implements ClassToInstanceMap<B>, Serializable {

  
  public static <B> MutableClassToInstanceMap<B> create() {
    return new MutableClassToInstanceMap<B>(new HashMap<Class<? extends B>, B>());
  }

  
  public static <B> MutableClassToInstanceMap<B> create(Map<Class<? extends B>, B> backingMap) {
    return new MutableClassToInstanceMap<B>(backingMap);
  }

  private final Map<Class<? extends B>, B> delegate;

  private MutableClassToInstanceMap(Map<Class<? extends B>, B> delegate) {
    this.delegate = checkNotNull(delegate);
  }

  @Override
  protected Map<Class<? extends B>, B> delegate() {
    return delegate;
  }

  static <B> Entry<Class<? extends B>, B> checkedEntry(final Entry<Class<? extends B>, B> entry) {
    return new ForwardingMapEntry<Class<? extends B>, B>() {
      @Override
      protected Entry<Class<? extends B>, B> delegate() {
        return entry;
      }

      @Override
      public B setValue(B value) {
        return super.setValue(cast(getKey(), value));
      }
    };
  }

  @Override
  public Set<Entry<Class<? extends B>, B>> entrySet() {
    return new ForwardingSet<Entry<Class<? extends B>, B>>() {

      @Override
      protected Set<Entry<Class<? extends B>, B>> delegate() {
        return MutableClassToInstanceMap.this.delegate().entrySet();
      }

      @Override
      public Iterator<Entry<Class<? extends B>, B>> iterator() {
        return new TransformedIterator<Entry<Class<? extends B>, B>, Entry<Class<? extends B>, B>>(
            delegate().iterator()) {
          @Override
          Entry<Class<? extends B>, B> transform(Entry<Class<? extends B>, B> from) {
            return checkedEntry(from);
          }
        };
      }

      @Override
      public Object[] toArray() {
        return standardToArray();
      }

      @Override
      public <T> T[] toArray(T[] array) {
        return standardToArray(array);
      }
    };
  }

  @Override
  @CanIgnoreReturnValue
  public B put(Class<? extends B> key, B value) {
    return super.put(key, cast(key, value));
  }

  @Override
  public void putAll(Map<? extends Class<? extends B>, ? extends B> map) {
    Map<Class<? extends B>, B> copy = new LinkedHashMap<>(map);
    for (Entry<? extends Class<? extends B>, B> entry : copy.entrySet()) {
      cast(entry.getKey(), entry.getValue());
    }
    super.putAll(copy);
  }

  @CanIgnoreReturnValue
  @Override
  public <T extends B> T putInstance(Class<T> type, T value) {
    return cast(type, put(type, value));
  }

  @Override
  public <T extends B> T getInstance(Class<T> type) {
    return cast(type, get(type));
  }

  @CanIgnoreReturnValue
  private static <B, T extends B> T cast(Class<T> type, B value) {
    return Primitives.wrap(type).cast(value);
  }

  private Object writeReplace() {
    return new SerializedForm(delegate());
  }

  
  private static final class SerializedForm<B> implements Serializable {
    private final Map<Class<? extends B>, B> backingMap;

    SerializedForm(Map<Class<? extends B>, B> backingMap) {
      this.backingMap = backingMap;
    }

    Object readResolve() {
      return create(backingMap);
    }

    private static final long serialVersionUID = 0;
  }
}
