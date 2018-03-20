
package com.iluwatar.interpreter;

import org.junit.jupiter.params.provider.Arguments;

import java.util.stream.Stream;


public class MultiplyExpressionTest extends ExpressionTest<MultiplyExpression> {

  
  @Override
  public Stream<Arguments> expressionProvider() {
    return prepareParameters((f, s) -> f * s);
  }

  
  public MultiplyExpressionTest() {
    super("*", MultiplyExpression::new);
  }

}