
package com.iluwatar.interpreter;

import org.junit.jupiter.params.ParameterizedTest;
import org.junit.jupiter.params.provider.Arguments;
import org.junit.jupiter.params.provider.MethodSource;

import java.util.stream.Stream;

import static org.junit.jupiter.api.Assertions.assertEquals;


public class NumberExpressionTest extends ExpressionTest<NumberExpression> {

  
  @Override
  public Stream<Arguments> expressionProvider() {
    return prepareParameters((f, s) -> f);
  }

  
  public NumberExpressionTest() {
    super("number", (f, s) -> f);
  }

  
  @ParameterizedTest
  @MethodSource("expressionProvider")
  public void testFromString(NumberExpression first) throws Exception {
    final int expectedValue = first.interpret();
    final String testStringValue = String.valueOf(expectedValue);
    final NumberExpression numberExpression = new NumberExpression(testStringValue);
    assertEquals(expectedValue, numberExpression.interpret());
  }

}