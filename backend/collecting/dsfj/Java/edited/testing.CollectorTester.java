

package com.google.common.testing;

import static com.google.common.base.Preconditions.checkNotNull;
import static junit.framework.Assert.assertTrue;

import com.google.common.annotations.Beta;
import com.google.common.annotations.GwtCompatible;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.EnumSet;
import java.util.List;
import java.util.Objects;
import java.util.function.BiPredicate;
import java.util.stream.Collector;
import org.checkerframework.checker.nullness.compatqual.NullableDecl;


@Beta
@GwtCompatible
public final class CollectorTester<T, A, R> {
  
  public static <T, A, R> CollectorTester<T, A, R> of(Collector<T, A, R> collector) {
    return of(collector, Objects::equals);
  }

  
  public static <T, A, R> CollectorTester<T, A, R> of(
      Collector<T, A, R> collector, BiPredicate<? super R, ? super R> equivalence) {
    return new CollectorTester<>(collector, equivalence);
  }

  private final Collector<T, A, R> collector;
  private final BiPredicate<? super R, ? super R> equivalence;

  private CollectorTester(
      Collector<T, A, R> collector, BiPredicate<? super R, ? super R> equivalence) {
    this.collector = checkNotNull(collector);
    this.equivalence = checkNotNull(equivalence);
  }

  
  enum CollectStrategy {
    
    SEQUENTIAL {
      @Override
      final <T, A, R> A result(Collector<T, A, R> collector, Iterable<T> inputs) {
        A accum = collector.supplier().get();
        for (T input : inputs) {
          collector.accumulator().accept(accum, input);
        }
        return accum;
      }
    },
    
    MERGE_LEFT_ASSOCIATIVE {
      @Override
      final <T, A, R> A result(Collector<T, A, R> collector, Iterable<T> inputs) {
        A accum = collector.supplier().get();
        for (T input : inputs) {
          A newAccum = collector.supplier().get();
          collector.accumulator().accept(newAccum, input);
          accum = collector.combiner().apply(accum, newAccum);
        }
        return accum;
      }
    },
    
    MERGE_RIGHT_ASSOCIATIVE {
      @Override
      final <T, A, R> A result(Collector<T, A, R> collector, Iterable<T> inputs) {
        List<A> stack = new ArrayList<>();
        for (T input : inputs) {
          A newAccum = collector.supplier().get();
          collector.accumulator().accept(newAccum, input);
          push(stack, newAccum);
        }
        push(stack, collector.supplier().get());
        while (stack.size() > 1) {
          A right = pop(stack);
          A left = pop(stack);
          push(stack, collector.combiner().apply(left, right));
        }
        return pop(stack);
      }

      <E> void push(List<E> stack, E value) {
        stack.add(value);
      }

      <E> E pop(List<E> stack) {
        return stack.remove(stack.size() - 1);
      }
    };

    abstract <T, A, R> A result(Collector<T, A, R> collector, Iterable<T> inputs);
  }

  
  @SafeVarargs
  public final CollectorTester<T, A, R> expectCollects(
      @NullableDecl R expectedResult, T... inputs) {
    List<T> list = Arrays.asList(inputs);
    doExpectCollects(expectedResult, list);
    if (collector.characteristics().contains(Collector.Characteristics.UNORDERED)) {
      Collections.reverse(list);
      doExpectCollects(expectedResult, list);
    }
    return this;
  }

  private void doExpectCollects(@NullableDecl R expectedResult, List<T> inputs) {
    for (CollectStrategy scheme : EnumSet.allOf(CollectStrategy.class)) {
      A finalAccum = scheme.result(collector, inputs);
      if (collector.characteristics().contains(Collector.Characteristics.IDENTITY_FINISH)) {
        assertEquivalent(expectedResult, (R) finalAccum);
      }
      assertEquivalent(expectedResult, collector.finisher().apply(finalAccum));
    }
  }

  private void assertEquivalent(@NullableDecl R expected, @NullableDecl R actual) {
    assertTrue(
        "Expected " + expected + " got " + actual + " modulo equivalence " + equivalence,
        equivalence.test(expected, actual));
  }
}
