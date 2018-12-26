

package com.google.common.collect;

import static com.google.common.base.Preconditions.checkNotNull;
import static com.google.common.collect.CollectPreconditions.checkNonnegative;

import com.google.common.annotations.Beta;
import com.google.common.annotations.GwtCompatible;
import com.google.common.base.Supplier;
import java.io.Serializable;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Comparator;
import java.util.EnumMap;
import java.util.EnumSet;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.SortedSet;
import java.util.TreeMap;
import java.util.TreeSet;


@Beta
@GwtCompatible
public abstract class MultimapBuilder<K0, V0> {
  

  private MultimapBuilder() {}

  private static final int DEFAULT_EXPECTED_KEYS = 8;

  
  public static MultimapBuilderWithKeys<Object> hashKeys() {
    return hashKeys(DEFAULT_EXPECTED_KEYS);
  }

  
  public static MultimapBuilderWithKeys<Object> hashKeys(final int expectedKeys) {
    checkNonnegative(expectedKeys, "expectedKeys");
    return new MultimapBuilderWithKeys<Object>() {
      @Override
      <K, V> Map<K, Collection<V>> createMap() {
        return Platform.newHashMapWithExpectedSize(expectedKeys);
      }
    };
  }

  
  public static MultimapBuilderWithKeys<Object> linkedHashKeys() {
    return linkedHashKeys(DEFAULT_EXPECTED_KEYS);
  }

  
  public static MultimapBuilderWithKeys<Object> linkedHashKeys(final int expectedKeys) {
    checkNonnegative(expectedKeys, "expectedKeys");
    return new MultimapBuilderWithKeys<Object>() {
      @Override
      <K, V> Map<K, Collection<V>> createMap() {
        return Platform.newLinkedHashMapWithExpectedSize(expectedKeys);
      }
    };
  }

  
  @SuppressWarnings("rawtypes")
  public static MultimapBuilderWithKeys<Comparable> treeKeys() {
    return treeKeys(Ordering.natural());
  }

  
  public static <K0> MultimapBuilderWithKeys<K0> treeKeys(final Comparator<K0> comparator) {
    checkNotNull(comparator);
    return new MultimapBuilderWithKeys<K0>() {
      @Override
      <K extends K0, V> Map<K, Collection<V>> createMap() {
        return new TreeMap<>(comparator);
      }
    };
  }

  
  public static <K0 extends Enum<K0>> MultimapBuilderWithKeys<K0> enumKeys(
      final Class<K0> keyClass) {
    checkNotNull(keyClass);
    return new MultimapBuilderWithKeys<K0>() {
      @SuppressWarnings("unchecked")
      @Override
      <K extends K0, V> Map<K, Collection<V>> createMap() {
                        return (Map<K, Collection<V>>) new EnumMap<K0, Collection<V>>(keyClass);
      }
    };
  }

  private static final class ArrayListSupplier<V> implements Supplier<List<V>>, Serializable {
    private final int expectedValuesPerKey;

    ArrayListSupplier(int expectedValuesPerKey) {
      this.expectedValuesPerKey = checkNonnegative(expectedValuesPerKey, "expectedValuesPerKey");
    }

    @Override
    public List<V> get() {
      return new ArrayList<V>(expectedValuesPerKey);
    }
  }

  private enum LinkedListSupplier implements Supplier<List<Object>> {
    INSTANCE;

    public static <V> Supplier<List<V>> instance() {
            @SuppressWarnings({"rawtypes", "unchecked"})
      Supplier<List<V>> result = (Supplier) INSTANCE;
      return result;
    }

    @Override
    public List<Object> get() {
      return new LinkedList<>();
    }
  }

  private static final class HashSetSupplier<V> implements Supplier<Set<V>>, Serializable {
    private final int expectedValuesPerKey;

    HashSetSupplier(int expectedValuesPerKey) {
      this.expectedValuesPerKey = checkNonnegative(expectedValuesPerKey, "expectedValuesPerKey");
    }

    @Override
    public Set<V> get() {
      return Platform.newHashSetWithExpectedSize(expectedValuesPerKey);
    }
  }
  
  private static final class LinkedHashSetSupplier<V> implements Supplier<Set<V>>, Serializable {
    private final int expectedValuesPerKey;

    LinkedHashSetSupplier(int expectedValuesPerKey) {
      this.expectedValuesPerKey = checkNonnegative(expectedValuesPerKey, "expectedValuesPerKey");
    }

    @Override
    public Set<V> get() {
      return Platform.newLinkedHashSetWithExpectedSize(expectedValuesPerKey);
    }
  }

  private static final class TreeSetSupplier<V> implements Supplier<SortedSet<V>>, Serializable {
    private final Comparator<? super V> comparator;

    TreeSetSupplier(Comparator<? super V> comparator) {
      this.comparator = checkNotNull(comparator);
    }

    @Override
    public SortedSet<V> get() {
      return new TreeSet<V>(comparator);
    }
  }

  private static final class EnumSetSupplier<V extends Enum<V>>
      implements Supplier<Set<V>>, Serializable {
    private final Class<V> clazz;

    EnumSetSupplier(Class<V> clazz) {
      this.clazz = checkNotNull(clazz);
    }

    @Override
    public Set<V> get() {
      return EnumSet.noneOf(clazz);
    }
  }

  
  public abstract static class MultimapBuilderWithKeys<K0> {

    private static final int DEFAULT_EXPECTED_VALUES_PER_KEY = 2;

    MultimapBuilderWithKeys() {}

    abstract <K extends K0, V> Map<K, Collection<V>> createMap();

    
    public ListMultimapBuilder<K0, Object> arrayListValues() {
      return arrayListValues(DEFAULT_EXPECTED_VALUES_PER_KEY);
    }

    
    public ListMultimapBuilder<K0, Object> arrayListValues(final int expectedValuesPerKey) {
      checkNonnegative(expectedValuesPerKey, "expectedValuesPerKey");
      return new ListMultimapBuilder<K0, Object>() {
        @Override
        public <K extends K0, V> ListMultimap<K, V> build() {
          return Multimaps.newListMultimap(
              MultimapBuilderWithKeys.this.<K, V>createMap(),
              new ArrayListSupplier<V>(expectedValuesPerKey));
        }
      };
    }

    
    public ListMultimapBuilder<K0, Object> linkedListValues() {
      return new ListMultimapBuilder<K0, Object>() {
        @Override
        public <K extends K0, V> ListMultimap<K, V> build() {
          return Multimaps.newListMultimap(
              MultimapBuilderWithKeys.this.<K, V>createMap(), LinkedListSupplier.<V>instance());
        }
      };
    }

    
    public SetMultimapBuilder<K0, Object> hashSetValues() {
      return hashSetValues(DEFAULT_EXPECTED_VALUES_PER_KEY);
    }

    
    public SetMultimapBuilder<K0, Object> hashSetValues(final int expectedValuesPerKey) {
      checkNonnegative(expectedValuesPerKey, "expectedValuesPerKey");
      return new SetMultimapBuilder<K0, Object>() {
        @Override
        public <K extends K0, V> SetMultimap<K, V> build() {
          return Multimaps.newSetMultimap(
              MultimapBuilderWithKeys.this.<K, V>createMap(),
              new HashSetSupplier<V>(expectedValuesPerKey));
        }
      };
    }

    
    public SetMultimapBuilder<K0, Object> linkedHashSetValues() {
      return linkedHashSetValues(DEFAULT_EXPECTED_VALUES_PER_KEY);
    }

    
    public SetMultimapBuilder<K0, Object> linkedHashSetValues(final int expectedValuesPerKey) {
      checkNonnegative(expectedValuesPerKey, "expectedValuesPerKey");
      return new SetMultimapBuilder<K0, Object>() {
        @Override
        public <K extends K0, V> SetMultimap<K, V> build() {
          return Multimaps.newSetMultimap(
              MultimapBuilderWithKeys.this.<K, V>createMap(),
              new LinkedHashSetSupplier<V>(expectedValuesPerKey));
        }
      };
    }

    
    @SuppressWarnings("rawtypes")
    public SortedSetMultimapBuilder<K0, Comparable> treeSetValues() {
      return treeSetValues(Ordering.natural());
    }

    
    public <V0> SortedSetMultimapBuilder<K0, V0> treeSetValues(final Comparator<V0> comparator) {
      checkNotNull(comparator, "comparator");
      return new SortedSetMultimapBuilder<K0, V0>() {
        @Override
        public <K extends K0, V extends V0> SortedSetMultimap<K, V> build() {
          return Multimaps.newSortedSetMultimap(
              MultimapBuilderWithKeys.this.<K, V>createMap(), new TreeSetSupplier<V>(comparator));
        }
      };
    }

    
    public <V0 extends Enum<V0>> SetMultimapBuilder<K0, V0> enumSetValues(
        final Class<V0> valueClass) {
      checkNotNull(valueClass, "valueClass");
      return new SetMultimapBuilder<K0, V0>() {
        @Override
        public <K extends K0, V extends V0> SetMultimap<K, V> build() {
                              @SuppressWarnings({"unchecked", "rawtypes"})
          Supplier<Set<V>> factory = (Supplier) new EnumSetSupplier<V0>(valueClass);
          return Multimaps.newSetMultimap(MultimapBuilderWithKeys.this.<K, V>createMap(), factory);
        }
      };
    }
  }

  
  public abstract <K extends K0, V extends V0> Multimap<K, V> build();

  
  public <K extends K0, V extends V0> Multimap<K, V> build(
      Multimap<? extends K, ? extends V> multimap) {
    Multimap<K, V> result = build();
    result.putAll(multimap);
    return result;
  }

  
  public abstract static class ListMultimapBuilder<K0, V0> extends MultimapBuilder<K0, V0> {
    ListMultimapBuilder() {}

    @Override
    public abstract <K extends K0, V extends V0> ListMultimap<K, V> build();

    @Override
    public <K extends K0, V extends V0> ListMultimap<K, V> build(
        Multimap<? extends K, ? extends V> multimap) {
      return (ListMultimap<K, V>) super.build(multimap);
    }
  }

  
  public abstract static class SetMultimapBuilder<K0, V0> extends MultimapBuilder<K0, V0> {
    SetMultimapBuilder() {}

    @Override
    public abstract <K extends K0, V extends V0> SetMultimap<K, V> build();

    @Override
    public <K extends K0, V extends V0> SetMultimap<K, V> build(
        Multimap<? extends K, ? extends V> multimap) {
      return (SetMultimap<K, V>) super.build(multimap);
    }
  }

  
  public abstract static class SortedSetMultimapBuilder<K0, V0> extends SetMultimapBuilder<K0, V0> {
    SortedSetMultimapBuilder() {}

    @Override
    public abstract <K extends K0, V extends V0> SortedSetMultimap<K, V> build();

    @Override
    public <K extends K0, V extends V0> SortedSetMultimap<K, V> build(
        Multimap<? extends K, ? extends V> multimap) {
      return (SortedSetMultimap<K, V>) super.build(multimap);
    }
  }
}
