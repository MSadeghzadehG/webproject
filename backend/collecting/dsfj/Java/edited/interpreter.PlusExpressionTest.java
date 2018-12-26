
package com.iluwatar.interpreter;

import org.junit.jupiter.params.provider.Arguments;

import java.util.stream.Stream;


public class PlusExpressionTest extends ExpressionTest<PlusExpression> {

  
  @Override
  public Stream<Arguments> expressionProvider() {
    return prepareParameters((f, s) -> f + s);
  }

  
  public PlusExpressionTest() {
    super("+", PlusExpression::new);
  }

}