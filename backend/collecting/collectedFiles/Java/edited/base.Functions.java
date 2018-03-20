

package com.google.common.base;

import static com.google.common.base.Preconditions.checkArgument;
import static com.google.common.base.Preconditions.checkNotNull;

import com.google.common.annotations.GwtCompatible;
import java.io.Serializable;
import java.util.Map;
import org.checkerframework.checker.nullness.compatqual.NullableDecl;


@GwtCompatible
public final class Functions {
  private Functions() {}

  
  public static Function<Object, String> toStringFunction() {
    return ToStringFunction.INSTANCE;
  }

    private enum ToStringFunction implements Function<Object, String> {
    INSTANCE;

    @Override
    public String apply(Object o) {
      checkNotNull(o);       return o.toString();
    }

    @Override
    public String toString() {
      return "Functions.toStringFunction()";
    }
  }

  
    @SuppressWarnings("unchecked")
  public static <E> Function<E, E> identity() {
    return (Function<E, E>) IdentityFunction.INSTANCE;
  }

    private enum IdentityFunction implements Function<Object, Object> {
    INSTANCE;

    @Override
    @NullableDecl
    public Object apply(@NullableDecl Object o) {
      return o;
    }

    @Override
    public String toString() {
      return "Functions.identity()";
    }
  }

  
  public static <K, V> Function<K, V> forMap(Map<K, V> map) {
    return new FunctionForMapNoDefault<>(map);
  }

  
  public static <K, V> Function<K, V> forMap(
      Map<K, ? extends V> map, @NullableDecl V defaultValue) {
    return new ForMapWithDefault<>(map, defaultValue);
  }

  private static class FunctionForMapNoDefault<K, V> implements Function<K, V>, Serializable {
    final Map<K, V> map;

    FunctionForMapNoDefault(Map<K, V> map) {
      this.map = checkNotNull(map);
    }

    @Override
    public V apply(@NullableDecl K key) {
      V result = map.get(key);
      checkArgument(result != null || map.containsKey(key), "Key '%s' not present in map", key);
      return result;
    }

    @Override
    public boolean equals(@NullableDecl Object o) {
      if (o instanceof FunctionForMapNoDefault) {
        FunctionForMapNoDefault<?, ?> that = (FunctionForMapNoDefault<?, ?>) o;
        return map.equals(that.map);
      }
      return false;
    }

    @Override
    public int hashCode() {
      return map.hashCode();
    }

    @Override
    public String toString() {
      return "Functions.forMap(" + map + ")";
    }

    private static final long serialVersionUID = 0;
  }

  private static class ForMapWithDefault<K, V> implements Function<K, V>, Serializable {
    final Map<K, ? extends V> map;
    @NullableDecl final V defaultValue;

    ForMapWithDefault(Map<K, ? extends V> map, @NullableDecl V defaultValue) {
      this.map = checkNotNull(map);
      this.defaultValue = defaultValue;
    }

    @Override
    public V apply(@NullableDecl K key) {
      V result = map.get(key);
      return (result != null || map.containsKey(key)) ? result : defaultValue;
    }

    @Override
    public boolean equals(@NullableDecl Object o) {
      if (o instanceof ForMapWithDefault) {
        ForMapWithDefault<?, ?> that = (ForMapWithDefault<?, ?>) o;
        return map.equals(that.map) && Objects.equal(defaultValue, that.defaultValue);
      }
      return false;
    }

    @Override
    public int hashCode() {
      return Objects.hashCode(map, defaultValue);
    }

    @Override
    public String toString() {
            return "Functions.forMap(" + map + ", defaultValue=" + defaultValue + ")";
    }

    private static final long serialVersionUID = 0;
  }

  
  public static <A, B, C> Function<A, C> compose(Function<B, C> g, Function<A, ? extends B> f) {
    return new FunctionComposition<>(g, f);
  }

  private static class FunctionComposition<A, B, C> implements Function<A, C>, Serializable {
    private final Function<B, C> g;
    private final Function<A, ? extends B> f;

    public FunctionComposition(Function<B, C> g, Function<A, ? extends B> f) {
      this.g = checkNotNull(g);
      this.f = checkNotNull(f);
    }

    @Override
    public C apply(@NullableDecl A a) {
      return g.apply(f.apply(a));
    }

    @Override
    public boolean equals(@NullableDecl Object obj) {
      if (obj instanceof FunctionComposition) {
        FunctionComposition<?, ?, ?> that = (FunctionComposition<?, ?, ?>) obj;
        return f.equals(that.f) && g.equals(that.g);
      }
      return false;
    }

    @Override
    public int hashCode() {
      return f.hashCode() ^ g.hashCode();
    }

    @Override
    public String toString() {
            return g + "(" + f + ")";
    }

    private static final long serialVersionUID = 0;
  }

  
  public static <T> Function<T, Boolean> forPredicate(Predicate<T> predicate) {
    return new PredicateFunction<T>(predicate);
  }

  
  private static class PredicateFunction<T> implements Function<T, Boolean>, Serializable {
    private final Predicate<T> predicate;

    private PredicateFunction(Predicate<T> predicate) {
      this.predicate = checkNotNull(predicate);
    }

    @Override
    public Boolean apply(@NullableDecl T t) {
      return predicate.apply(t);
    }

    @Override
    public boolean equals(@NullableDecl Object obj) {
      if (obj instanceof PredicateFunction) {
        PredicateFunction<?> that = (PredicateFunction<?>) obj;
        return predicate.equals(that.predicate);
      }
      return false;
    }

    @Override
    public int hashCode() {
      return predicate.hashCode();
    }

    @Override
    public String toString() {
      return "Functions.forPredicate(" + predicate + ")";
    }

    private static final long serialVersionUID = 0;
  }

  
  public static <E> Function<Object, E> constant(@NullableDecl E value) {
    return new ConstantFunction<E>(value);
  }

  private static class ConstantFunction<E> implements Function<Object, E>, Serializable {
    @NullableDecl private final E value;

    public ConstantFunction(@NullableDecl E value) {
      this.value = value;
    }

    @Override
    public E apply(@NullableDecl Object from) {
      return value;
    }

    @Override
    public boolean equals(@NullableDecl Object obj) {
      if (obj instanceof ConstantFunction) {
        ConstantFunction<?> that = (ConstantFunction<?>) obj;
        return Objects.equal(value, that.value);
      }
      return false;
    }

    @Override
    public int hashCode() {
      return (value == null) ? 0 : value.hashCode();
    }

    @Override
    public String toString() {
      return "Functions.constant(" + value + ")";
    }

    private static final long serialVersionUID = 0;
  }

  
  public static <T> Function<Object, T> forSupplier(Supplier<T> supplier) {
    return new SupplierFunction<T>(supplier);
  }

  
  private static class SupplierFunction<T> implements Function<Object, T>, Serializable {

    private final Supplier<T> supplier;

    private SupplierFunction(Supplier<T> supplier) {
      this.supplier = checkNotNull(supplier);
    }

    @Override
    public T apply(@NullableDecl Object input) {
      return supplier.get();
    }

    @Override
    public boolean equals(@NullableDecl Object obj) {
      if (obj instanceof SupplierFunction) {
        SupplierFunction<?> that = (SupplierFunction<?>) obj;
        return this.supplier.equals(that.supplier);
      }
      return false;
    }

    @Override
    public int hashCode() {
      return supplier.hashCode();
    }

    @Override
    public String toString() {
      return "Functions.forSupplier(" + supplier + ")";
    }

    private static final long serialVersionUID = 0;
  }
}
