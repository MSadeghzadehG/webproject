
package com.iluwatar.intercepting.filter;

import org.junit.jupiter.params.ParameterizedTest;
import org.junit.jupiter.params.provider.MethodSource;

import java.util.ArrayList;
import java.util.List;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertNotNull;
import static org.junit.jupiter.api.Assertions.assertNull;
import static org.junit.jupiter.api.Assertions.assertSame;


public class FilterTest {

  private static final Order PERFECT_ORDER = new Order("name", "12345678901", "addr", "dep", "order");
  private static final Order WRONG_ORDER = new Order("name", "12345678901", "addr", "dep", "");
  private static final Order WRONG_DEPOSIT = new Order("name", "12345678901", "addr", "", "order");
  private static final Order WRONG_ADDRESS = new Order("name", "12345678901", "", "dep", "order");
  private static final Order WRONG_CONTACT = new Order("name", "", "addr", "dep", "order");
  private static final Order WRONG_NAME = new Order("", "12345678901", "addr", "dep", "order");

  static List<Object[]> getTestData() {
    final List<Object[]> testData = new ArrayList<>();
    testData.add(new Object[]{new NameFilter(), PERFECT_ORDER, ""});
    testData.add(new Object[]{new NameFilter(), WRONG_NAME, "Invalid name!"});
    testData.add(new Object[]{new NameFilter(), WRONG_CONTACT, ""});
    testData.add(new Object[]{new NameFilter(), WRONG_ADDRESS, ""});
    testData.add(new Object[]{new NameFilter(), WRONG_DEPOSIT, ""});
    testData.add(new Object[]{new NameFilter(), WRONG_ORDER, ""});

    testData.add(new Object[]{new ContactFilter(), PERFECT_ORDER, ""});
    testData.add(new Object[]{new ContactFilter(), WRONG_NAME, ""});
    testData.add(new Object[]{new ContactFilter(), WRONG_CONTACT, "Invalid contact number!"});
    testData.add(new Object[]{new ContactFilter(), WRONG_ADDRESS, ""});
    testData.add(new Object[]{new ContactFilter(), WRONG_DEPOSIT, ""});
    testData.add(new Object[]{new ContactFilter(), WRONG_ORDER, ""});

    testData.add(new Object[]{new AddressFilter(), PERFECT_ORDER, ""});
    testData.add(new Object[]{new AddressFilter(), WRONG_NAME, ""});
    testData.add(new Object[]{new AddressFilter(), WRONG_CONTACT, ""});
    testData.add(new Object[]{new AddressFilter(), WRONG_ADDRESS, "Invalid address!"});
    testData.add(new Object[]{new AddressFilter(), WRONG_DEPOSIT, ""});
    testData.add(new Object[]{new AddressFilter(), WRONG_ORDER, ""});

    testData.add(new Object[]{new DepositFilter(), PERFECT_ORDER, ""});
    testData.add(new Object[]{new DepositFilter(), WRONG_NAME, ""});
    testData.add(new Object[]{new DepositFilter(), WRONG_CONTACT, ""});
    testData.add(new Object[]{new DepositFilter(), WRONG_ADDRESS, ""});
    testData.add(new Object[]{new DepositFilter(), WRONG_DEPOSIT, "Invalid deposit number!"});
    testData.add(new Object[]{new DepositFilter(), WRONG_ORDER, ""});

    testData.add(new Object[]{new OrderFilter(), PERFECT_ORDER, ""});
    testData.add(new Object[]{new OrderFilter(), WRONG_NAME, ""});
    testData.add(new Object[]{new OrderFilter(), WRONG_CONTACT, ""});
    testData.add(new Object[]{new OrderFilter(), WRONG_ADDRESS, ""});
    testData.add(new Object[]{new OrderFilter(), WRONG_DEPOSIT, ""});
    testData.add(new Object[]{new OrderFilter(), WRONG_ORDER, "Invalid order!"});

    return testData;
  }

  @ParameterizedTest
  @MethodSource("getTestData")
  public void testExecute(Filter filter, Order order, String expectedResult) throws Exception {
    final String result = filter.execute(order);
    assertNotNull(result);
    assertEquals(expectedResult, result.trim());
  }

  @ParameterizedTest
  @MethodSource("getTestData")
  public void testNext(Filter filter) throws Exception {
    assertNull(filter.getNext());
    assertSame(filter, filter.getLast());
  }

}
