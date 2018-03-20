
package com.iluwatar.visitor;

import org.junit.jupiter.api.Test;

import java.util.Arrays;
import java.util.function.Function;

import static org.mockito.Matchers.eq;
import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.verifyNoMoreInteractions;


public abstract class UnitTest<U extends Unit> {

  
  private final Function<Unit[], U> factory;

  
  public UnitTest(final Function<Unit[], U> factory) {
    this.factory = factory;
  }

  @Test
  public void testAccept() throws Exception {
    final Unit[] children = new Unit[5];
    Arrays.setAll(children, (i) -> mock(Unit.class));

    final U unit = this.factory.apply(children);
    final UnitVisitor visitor = mock(UnitVisitor.class);
    unit.accept(visitor);
    verifyVisit(unit, visitor);

    for (final Unit child : children) {
      verify(child).accept(eq(visitor));
    }

    verifyNoMoreInteractions(children);
    verifyNoMoreInteractions(visitor);
  }

  
  abstract void verifyVisit(final U unit, final UnitVisitor mockedVisitor);

}
