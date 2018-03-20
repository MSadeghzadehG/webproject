
package com.iluwatar.interpreter;

import org.junit.jupiter.api.Disabled;
import org.junit.jupiter.api.TestInstance;
import org.junit.jupiter.params.ParameterizedTest;
import org.junit.jupiter.params.provider.Arguments;
import org.junit.jupiter.params.provider.MethodSource;

import java.util.ArrayList;
import java.util.List;
import java.util.function.BiFunction;
import java.util.function.IntBinaryOperator;
import java.util.stream.Stream;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertNotNull;


@TestInstance(TestInstance.Lifecycle.PER_CLASS)
public abstract class ExpressionTest<E extends Expression> {

  
  static Stream<Arguments> prepareParameters(final IntBinaryOperator resultCalc) {
    final List<Arguments> testData = new ArrayList<>();
    for (int i = -10; i < 10; i++) {
      for (int j = -10; j < 10; j++) {
        testData.add(Arguments.of(
                new NumberExpression(i),
                new NumberExpression(j),
                resultCalc.applyAsInt(i, j)
        ));
      }
    }
    return testData.stream();
  }

  
  private final String expectedToString;

  
  private final BiFunction<NumberExpression, NumberExpression, E> factory;

  
  ExpressionTest(final String expectedToString,
                 final BiFunction<NumberExpression, NumberExpression, E> factory
  ) {
    this.expectedToString = expectedToString;
    this.factory = factory;
  }

  
  public abstract Stream<Arguments> expressionProvider();

  
  @ParameterizedTest
  @MethodSource("expressionProvider")
  public void testInterpret(NumberExpression first, NumberExpression second, int result) {
    final E expression = factory.apply(first, second);
    assertNotNull(expression);
    assertEquals(result, expression.interpret());
  }

  
  @ParameterizedTest
  @MethodSource("expressionProvider")
  public void testToString(NumberExpression first, NumberExpression second) {
    final E expression = factory.apply(first, second);
    assertNotNull(expression);
    assertEquals(expectedToString, expression.toString());
  }
}
