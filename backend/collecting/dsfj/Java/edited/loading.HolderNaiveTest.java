
package com.iluwatar.lazy.loading;

import java.lang.reflect.Field;


public class HolderNaiveTest extends AbstractHolderTest {

  private final HolderNaive holder = new HolderNaive();

  @Override
  Heavy getInternalHeavyValue() throws Exception {
    final Field holderField = HolderNaive.class.getDeclaredField("heavy");
    holderField.setAccessible(true);
    return (Heavy) holderField.get(this.holder);
  }

  @Override
  Heavy getHeavy() {
    return holder.getHeavy();
  }

}