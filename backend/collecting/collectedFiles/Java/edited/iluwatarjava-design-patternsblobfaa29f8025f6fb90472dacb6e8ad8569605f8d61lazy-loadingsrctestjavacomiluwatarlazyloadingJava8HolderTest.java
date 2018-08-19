
package com.iluwatar.lazy.loading;

import java.lang.reflect.Field;
import java.util.function.Supplier;


public class Java8HolderTest extends AbstractHolderTest {

  private final Java8Holder holder = new Java8Holder();


  @Override
  Heavy getInternalHeavyValue() throws Exception {
    final Field holderField = Java8Holder.class.getDeclaredField("heavy");
    holderField.setAccessible(true);

    final Supplier<Heavy> supplier = (Supplier<Heavy>) holderField.get(this.holder);
    final Class<? extends Supplier> supplierClass = supplier.getClass();

            if (supplierClass.isLocalClass()) {
      final Field instanceField = supplierClass.getDeclaredField("heavyInstance");
      instanceField.setAccessible(true);
      return (Heavy) instanceField.get(supplier);
    } else {
      return null;
    }
  }

  @Override
  Heavy getHeavy() throws Exception {
    return holder.getHeavy();
  }

}