

package com.google.common.collect;

import static com.google.common.base.Preconditions.checkNotNull;
import static com.google.common.base.Preconditions.checkState;

import com.google.common.annotations.Beta;
import com.google.common.annotations.GwtCompatible;
import com.google.common.math.LongMath;
import java.util.ArrayDeque;
import java.util.Collection;
import java.util.Deque;
import java.util.Iterator;
import java.util.OptionalDouble;
import java.util.OptionalInt;
import java.util.OptionalLong;
import java.util.PrimitiveIterator;
import java.util.Spliterator;
import java.util.Spliterators;
import java.util.Spliterators.AbstractSpliterator;
import java.util.function.BiConsumer;
import java.util.function.BiFunction;
import java.util.function.Consumer;
import java.util.function.DoubleConsumer;
import java.util.function.IntConsumer;
import java.util.function.LongConsumer;
import java.util.stream.DoubleStream;
import java.util.stream.IntStream;
import java.util.stream.LongStream;
import java.util.stream.Stream;
import java.util.stream.StreamSupport;
import org.checkerframework.checker.nullness.compatqual.NullableDecl;


@Beta
@GwtCompatible
public final class Streams {
  
  public static <T> Stream<T> stream(Iterable<T> iterable) {
    return (iterable instanceof Collection)
        ? ((Collection<T>) iterable).stream()
        : StreamSupport.stream(iterable.spliterator(), false);
  }

  
  @Deprecated
  public static <T> Stream<T> stream(Collection<T> collection) {
    return collection.stream();
  }

  
  public static <T> Stream<T> stream(Iterator<T> iterator) {
    return StreamSupport.stream(Spliterators.spliteratorUnknownSize(iterator, 0), false);
  }

  
  public static <T> Stream<T> stream(com.google.common.base.Optional<T> optional) {
    return optional.isPresent() ? Stream.of(optional.get()) : Stream.of();
  }

  
  public static <T> Stream<T> stream(java.util.Optional<T> optional) {
    return optional.isPresent() ? Stream.of(optional.get()) : Stream.of();
  }

  
  public static IntStream stream(OptionalInt optional) {
    return optional.isPresent() ? IntStream.of(optional.getAsInt()) : IntStream.empty();
  }

  
  public static LongStream stream(OptionalLong optional) {
    return optional.isPresent() ? LongStream.of(optional.getAsLong()) : LongStream.empty();
  }

  
  public static DoubleStream stream(OptionalDouble optional) {
    return optional.isPresent() ? DoubleStream.of(optional.getAsDouble()) : DoubleStream.empty();
  }

  
  @SafeVarargs
  public static <T> Stream<T> concat(Stream<? extends T>... streams) {
        boolean isParallel = false;
    int characteristics = Spliterator.ORDERED | Spliterator.SIZED | Spliterator.NONNULL;
    long estimatedSize = 0L;
    ImmutableList.Builder<Spliterator<? extends T>> splitrsBuilder =
        new ImmutableList.Builder<>(streams.length);
    for (Stream<? extends T> stream : streams) {
      isParallel |= stream.isParallel();
      Spliterator<? extends T> splitr = stream.spliterator();
      splitrsBuilder.add(splitr);
      characteristics &= splitr.characteristics();
      estimatedSize = LongMath.saturatedAdd(estimatedSize, splitr.estimateSize());
    }
    return StreamSupport.stream(
            CollectSpliterators.flatMap(
                splitrsBuilder.build().spliterator(),
                splitr -> (Spliterator<T>) splitr,
                characteristics,
                estimatedSize),
            isParallel)
        .onClose(
            () -> {
              for (Stream<? extends T> stream : streams) {
                stream.close();
              }
            });
  }

  
  public static IntStream concat(IntStream... streams) {
        return Stream.of(streams).flatMapToInt(stream -> stream);
  }

  
  public static LongStream concat(LongStream... streams) {
        return Stream.of(streams).flatMapToLong(stream -> stream);
  }

  
  public static DoubleStream concat(DoubleStream... streams) {
        return Stream.of(streams).flatMapToDouble(stream -> stream);
  }

  
  public static <A, B, R> Stream<R> zip(
      Stream<A> streamA, Stream<B> streamB, BiFunction<? super A, ? super B, R> function) {
    checkNotNull(streamA);
    checkNotNull(streamB);
    checkNotNull(function);
    boolean isParallel = streamA.isParallel() || streamB.isParallel();     Spliterator<A> splitrA = streamA.spliterator();
    Spliterator<B> splitrB = streamB.spliterator();
    int characteristics =
        splitrA.characteristics()
            & splitrB.characteristics()
            & (Spliterator.SIZED | Spliterator.ORDERED);
    Iterator<A> itrA = Spliterators.iterator(splitrA);
    Iterator<B> itrB = Spliterators.iterator(splitrB);
    return StreamSupport.stream(
            new AbstractSpliterator<R>(
                Math.min(splitrA.estimateSize(), splitrB.estimateSize()), characteristics) {
              @Override
              public boolean tryAdvance(Consumer<? super R> action) {
                if (itrA.hasNext() && itrB.hasNext()) {
                  action.accept(function.apply(itrA.next(), itrB.next()));
                  return true;
                }
                return false;
              }
            },
            isParallel)
        .onClose(streamA::close)
        .onClose(streamB::close);
  }

  
  public static <A, B> void forEachPair(
      Stream<A> streamA, Stream<B> streamB, BiConsumer<? super A, ? super B> consumer) {
    checkNotNull(consumer);

    if (streamA.isParallel() || streamB.isParallel()) {
      zip(streamA, streamB, TemporaryPair::new).forEach(pair -> consumer.accept(pair.a, pair.b));
    } else {
      Iterator<A> iterA = streamA.iterator();
      Iterator<B> iterB = streamB.iterator();
      while (iterA.hasNext() && iterB.hasNext()) {
        consumer.accept(iterA.next(), iterB.next());
      }
    }
  }

    private static class TemporaryPair<A, B> {
    final A a;
    final B b;

    TemporaryPair(A a, B b) {
      this.a = a;
      this.b = b;
    }
  }

  
  public static <T, R> Stream<R> mapWithIndex(
      Stream<T> stream, FunctionWithIndex<? super T, ? extends R> function) {
    checkNotNull(stream);
    checkNotNull(function);
    boolean isParallel = stream.isParallel();
    Spliterator<T> fromSpliterator = stream.spliterator();

    if (!fromSpliterator.hasCharacteristics(Spliterator.SUBSIZED)) {
      Iterator<T> fromIterator = Spliterators.iterator(fromSpliterator);
      return StreamSupport.stream(
              new AbstractSpliterator<R>(
                  fromSpliterator.estimateSize(),
                  fromSpliterator.characteristics() & (Spliterator.ORDERED | Spliterator.SIZED)) {
                long index = 0;

                @Override
                public boolean tryAdvance(Consumer<? super R> action) {
                  if (fromIterator.hasNext()) {
                    action.accept(function.apply(fromIterator.next(), index++));
                    return true;
                  }
                  return false;
                }
              },
              isParallel)
          .onClose(stream::close);
    }
    class Splitr extends MapWithIndexSpliterator<Spliterator<T>, R, Splitr> implements Consumer<T> {
      @NullableDecl T holder;

      Splitr(Spliterator<T> splitr, long index) {
        super(splitr, index);
      }

      @Override
      public void accept(@NullableDecl T t) {
        this.holder = t;
      }

      @Override
      public boolean tryAdvance(Consumer<? super R> action) {
        if (fromSpliterator.tryAdvance(this)) {
          try {
            action.accept(function.apply(holder, index++));
            return true;
          } finally {
            holder = null;
          }
        }
        return false;
      }

      @Override
      Splitr createSplit(Spliterator<T> from, long i) {
        return new Splitr(from, i);
      }
    }
    return StreamSupport.stream(new Splitr(fromSpliterator, 0), isParallel).onClose(stream::close);
  }

  
  public static <R> Stream<R> mapWithIndex(IntStream stream, IntFunctionWithIndex<R> function) {
    checkNotNull(stream);
    checkNotNull(function);
    boolean isParallel = stream.isParallel();
    Spliterator.OfInt fromSpliterator = stream.spliterator();

    if (!fromSpliterator.hasCharacteristics(Spliterator.SUBSIZED)) {
      PrimitiveIterator.OfInt fromIterator = Spliterators.iterator(fromSpliterator);
      return StreamSupport.stream(
              new AbstractSpliterator<R>(
                  fromSpliterator.estimateSize(),
                  fromSpliterator.characteristics() & (Spliterator.ORDERED | Spliterator.SIZED)) {
                long index = 0;

                @Override
                public boolean tryAdvance(Consumer<? super R> action) {
                  if (fromIterator.hasNext()) {
                    action.accept(function.apply(fromIterator.nextInt(), index++));
                    return true;
                  }
                  return false;
                }
              },
              isParallel)
          .onClose(stream::close);
    }
    class Splitr extends MapWithIndexSpliterator<Spliterator.OfInt, R, Splitr>
        implements IntConsumer, Spliterator<R> {
      int holder;

      Splitr(Spliterator.OfInt splitr, long index) {
        super(splitr, index);
      }

      @Override
      public void accept(int t) {
        this.holder = t;
      }

      @Override
      public boolean tryAdvance(Consumer<? super R> action) {
        if (fromSpliterator.tryAdvance(this)) {
          action.accept(function.apply(holder, index++));
          return true;
        }
        return false;
      }

      @Override
      Splitr createSplit(Spliterator.OfInt from, long i) {
        return new Splitr(from, i);
      }
    }
    return StreamSupport.stream(new Splitr(fromSpliterator, 0), isParallel).onClose(stream::close);
  }

  
  public static <R> Stream<R> mapWithIndex(LongStream stream, LongFunctionWithIndex<R> function) {
    checkNotNull(stream);
    checkNotNull(function);
    boolean isParallel = stream.isParallel();
    Spliterator.OfLong fromSpliterator = stream.spliterator();

    if (!fromSpliterator.hasCharacteristics(Spliterator.SUBSIZED)) {
      PrimitiveIterator.OfLong fromIterator = Spliterators.iterator(fromSpliterator);
      return StreamSupport.stream(
              new AbstractSpliterator<R>(
                  fromSpliterator.estimateSize(),
                  fromSpliterator.characteristics() & (Spliterator.ORDERED | Spliterator.SIZED)) {
                long index = 0;

                @Override
                public boolean tryAdvance(Consumer<? super R> action) {
                  if (fromIterator.hasNext()) {
                    action.accept(function.apply(fromIterator.nextLong(), index++));
                    return true;
                  }
                  return false;
                }
              },
              isParallel)
          .onClose(stream::close);
    }
    class Splitr extends MapWithIndexSpliterator<Spliterator.OfLong, R, Splitr>
        implements LongConsumer, Spliterator<R> {
      long holder;

      Splitr(Spliterator.OfLong splitr, long index) {
        super(splitr, index);
      }

      @Override
      public void accept(long t) {
        this.holder = t;
      }

      @Override
      public boolean tryAdvance(Consumer<? super R> action) {
        if (fromSpliterator.tryAdvance(this)) {
          action.accept(function.apply(holder, index++));
          return true;
        }
        return false;
      }

      @Override
      Splitr createSplit(Spliterator.OfLong from, long i) {
        return new Splitr(from, i);
      }
    }
    return StreamSupport.stream(new Splitr(fromSpliterator, 0), isParallel).onClose(stream::close);
  }

  
  public static <R> Stream<R> mapWithIndex(
      DoubleStream stream, DoubleFunctionWithIndex<R> function) {
    checkNotNull(stream);
    checkNotNull(function);
    boolean isParallel = stream.isParallel();
    Spliterator.OfDouble fromSpliterator = stream.spliterator();

    if (!fromSpliterator.hasCharacteristics(Spliterator.SUBSIZED)) {
      PrimitiveIterator.OfDouble fromIterator = Spliterators.iterator(fromSpliterator);
      return StreamSupport.stream(
              new AbstractSpliterator<R>(
                  fromSpliterator.estimateSize(),
                  fromSpliterator.characteristics() & (Spliterator.ORDERED | Spliterator.SIZED)) {
                long index = 0;

                @Override
                public boolean tryAdvance(Consumer<? super R> action) {
                  if (fromIterator.hasNext()) {
                    action.accept(function.apply(fromIterator.nextDouble(), index++));
                    return true;
                  }
                  return false;
                }
              },
              isParallel)
          .onClose(stream::close);
    }
    class Splitr extends MapWithIndexSpliterator<Spliterator.OfDouble, R, Splitr>
        implements DoubleConsumer, Spliterator<R> {
      double holder;

      Splitr(Spliterator.OfDouble splitr, long index) {
        super(splitr, index);
      }

      @Override
      public void accept(double t) {
        this.holder = t;
      }

      @Override
      public boolean tryAdvance(Consumer<? super R> action) {
        if (fromSpliterator.tryAdvance(this)) {
          action.accept(function.apply(holder, index++));
          return true;
        }
        return false;
      }

      @Override
      Splitr createSplit(Spliterator.OfDouble from, long i) {
        return new Splitr(from, i);
      }
    }
    return StreamSupport.stream(new Splitr(fromSpliterator, 0), isParallel).onClose(stream::close);
  }

  
  @Beta
  public interface FunctionWithIndex<T, R> {
    
    R apply(T from, long index);
  }

  private abstract static class MapWithIndexSpliterator<
          F extends Spliterator<?>, R, S extends MapWithIndexSpliterator<F, R, S>>
      implements Spliterator<R> {
    final F fromSpliterator;
    long index;

    MapWithIndexSpliterator(F fromSpliterator, long index) {
      this.fromSpliterator = fromSpliterator;
      this.index = index;
    }

    abstract S createSplit(F from, long i);

    @Override
    public S trySplit() {
      @SuppressWarnings("unchecked")
      F split = (F) fromSpliterator.trySplit();
      if (split == null) {
        return null;
      }
      S result = createSplit(split, index);
      this.index += split.getExactSizeIfKnown();
      return result;
    }

    @Override
    public long estimateSize() {
      return fromSpliterator.estimateSize();
    }

    @Override
    public int characteristics() {
      return fromSpliterator.characteristics()
          & (Spliterator.ORDERED | Spliterator.SIZED | Spliterator.SUBSIZED);
    }
  }

  
  @Beta
  public interface IntFunctionWithIndex<R> {
    
    R apply(int from, long index);
  }

  
  @Beta
  public interface LongFunctionWithIndex<R> {
    
    R apply(long from, long index);
  }

  
  @Beta
  public interface DoubleFunctionWithIndex<R> {
    
    R apply(double from, long index);
  }

  
  public static <T> java.util.Optional<T> findLast(Stream<T> stream) {
    class OptionalState {
      boolean set = false;
      T value = null;

      void set(@NullableDecl T value) {
        this.set = true;
        this.value = value;
      }

      T get() {
        checkState(set);
        return value;
      }
    }
    OptionalState state = new OptionalState();

    Deque<Spliterator<T>> splits = new ArrayDeque<>();
    splits.addLast(stream.spliterator());

    while (!splits.isEmpty()) {
      Spliterator<T> spliterator = splits.removeLast();

      if (spliterator.getExactSizeIfKnown() == 0) {
        continue;       }

                  if (spliterator.hasCharacteristics(Spliterator.SUBSIZED)) {
                while (true) {
          Spliterator<T> prefix = spliterator.trySplit();
          if (prefix == null || prefix.getExactSizeIfKnown() == 0) {
            break;
          } else if (spliterator.getExactSizeIfKnown() == 0) {
            spliterator = prefix;
            break;
          }
        }

                spliterator.forEachRemaining(state::set);
        return java.util.Optional.of(state.get());
      }

      Spliterator<T> prefix = spliterator.trySplit();
      if (prefix == null || prefix.getExactSizeIfKnown() == 0) {
                spliterator.forEachRemaining(state::set);
        if (state.set) {
          return java.util.Optional.of(state.get());
        }
                continue;
      }
      splits.addLast(prefix);
      splits.addLast(spliterator);
    }
    return java.util.Optional.empty();
  }

  
  public static OptionalInt findLast(IntStream stream) {
        java.util.Optional<Integer> boxedLast = findLast(stream.boxed());
    return boxedLast.isPresent() ? OptionalInt.of(boxedLast.get()) : OptionalInt.empty();
  }

  
  public static OptionalLong findLast(LongStream stream) {
        java.util.Optional<Long> boxedLast = findLast(stream.boxed());
    return boxedLast.isPresent() ? OptionalLong.of(boxedLast.get()) : OptionalLong.empty();
  }

  
  public static OptionalDouble findLast(DoubleStream stream) {
        java.util.Optional<Double> boxedLast = findLast(stream.boxed());
    return boxedLast.isPresent() ? OptionalDouble.of(boxedLast.get()) : OptionalDouble.empty();
  }

  private Streams() {}
}
