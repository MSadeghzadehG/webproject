
package com.iluwatar.lazy.loading;

import org.junit.jupiter.api.Test;

import static java.time.Duration.ofMillis;
import static org.junit.jupiter.api.Assertions.assertNotNull;
import static org.junit.jupiter.api.Assertions.assertNull;
import static org.junit.jupiter.api.Assertions.assertSame;
import static org.junit.jupiter.api.Assertions.assertTimeout;


public abstract class AbstractHolderTest {

  
  abstract Heavy getInternalHeavyValue() throws Exception;

  
  abstract Heavy getHeavy() throws Exception;

  
  @Test
  public void testGetHeavy() throws Exception {
    assertTimeout(ofMillis(3000), () -> {
      assertNull(getInternalHeavyValue());
      assertNotNull(getHeavy());
      assertNotNull(getInternalHeavyValue());
      assertSame(getHeavy(), getInternalHeavyValue());
    });
  }

}
