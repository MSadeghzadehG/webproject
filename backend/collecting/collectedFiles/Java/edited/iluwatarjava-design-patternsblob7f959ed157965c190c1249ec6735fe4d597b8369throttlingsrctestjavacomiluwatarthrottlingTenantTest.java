
package com.iluwatar.throttling;

import org.junit.jupiter.api.Test;

import java.security.InvalidParameterException;

import static org.junit.jupiter.api.Assertions.assertThrows;


public class TenantTest {

  @Test
  public void constructorTest() {
    assertThrows(InvalidParameterException.class, () -> {
      Tenant tenant = new Tenant("FailTenant", -1);
    });
  }
}
