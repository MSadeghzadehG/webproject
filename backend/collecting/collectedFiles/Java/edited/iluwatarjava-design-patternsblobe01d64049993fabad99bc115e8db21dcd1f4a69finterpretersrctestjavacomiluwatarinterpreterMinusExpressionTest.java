
package com.iluwatar.interpreter;

import org.junit.jupiter.params.provider.Arguments;

import java.util.stream.Stream;


public class MinusExpressionTest extends ExpressionTest<MinusExpression> {

  
  @Override
  public Stream<Arguments> expressionProvider() {
    return prepareParameters((f, s) -> f - s);
  }

  
  public MinusExpressionTest() {
    super("-", MinusExpression::new);
  }

}