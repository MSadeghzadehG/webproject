

package com.iluwatar.dao;

import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.Nested;
import org.junit.jupiter.api.Test;

import java.util.Optional;
import java.util.stream.Stream;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertFalse;
import static org.junit.jupiter.api.Assertions.assertTrue;
import static org.junit.jupiter.api.Assumptions.assumeTrue;


public class InMemoryCustomerDaoTest {

  private InMemoryCustomerDao dao;
  private static final Customer CUSTOMER = new Customer(1, "Freddy", "Krueger");

  @BeforeEach
  public void setUp() {
    dao = new InMemoryCustomerDao();
    assertTrue(dao.add(CUSTOMER));
  }

  
  @Nested
  public class NonExistingCustomer {

    @Test
    public void addingShouldResultInSuccess() throws Exception {
      try (Stream<Customer> allCustomers = dao.getAll()) {
        assumeTrue(allCustomers.count() == 1);
      }

      final Customer nonExistingCustomer = new Customer(2, "Robert", "Englund");
      boolean result = dao.add(nonExistingCustomer);
      assertTrue(result);

      assertCustomerCountIs(2);
      assertEquals(nonExistingCustomer, dao.getById(nonExistingCustomer.getId()).get());
    }

    @Test
    public void deletionShouldBeFailureAndNotAffectExistingCustomers() throws Exception {
      final Customer nonExistingCustomer = new Customer(2, "Robert", "Englund");
      boolean result = dao.delete(nonExistingCustomer);

      assertFalse(result);
      assertCustomerCountIs(1);
    }

    @Test
    public void updationShouldBeFailureAndNotAffectExistingCustomers() throws Exception {
      final int nonExistingId = getNonExistingCustomerId();
      final String newFirstname = "Douglas";
      final String newLastname = "MacArthur";
      final Customer customer = new Customer(nonExistingId, newFirstname, newLastname);
      boolean result = dao.update(customer);

      assertFalse(result);
      assertFalse(dao.getById(nonExistingId).isPresent());
    }

    @Test
    public void retrieveShouldReturnNoCustomer() throws Exception {
      assertFalse(dao.getById(getNonExistingCustomerId()).isPresent());
    }
  }

  
  @Nested
  public class ExistingCustomer {

    @Test
    public void addingShouldResultInFailureAndNotAffectExistingCustomers() throws Exception {
      boolean result = dao.add(CUSTOMER);

      assertFalse(result);
      assertCustomerCountIs(1);
      assertEquals(CUSTOMER, dao.getById(CUSTOMER.getId()).get());
    }

    @Test
    public void deletionShouldBeSuccessAndCustomerShouldBeNonAccessible() throws Exception {
      boolean result = dao.delete(CUSTOMER);

      assertTrue(result);
      assertCustomerCountIs(0);
      assertFalse(dao.getById(CUSTOMER.getId()).isPresent());
    }

    @Test
    public void updationShouldBeSuccessAndAccessingTheSameCustomerShouldReturnUpdatedInformation() throws Exception {
      final String newFirstname = "Bernard";
      final String newLastname = "Montgomery";
      final Customer customer = new Customer(CUSTOMER.getId(), newFirstname, newLastname);
      boolean result = dao.update(customer);

      assertTrue(result);

      final Customer cust = dao.getById(CUSTOMER.getId()).get();
      assertEquals(newFirstname, cust.getFirstName());
      assertEquals(newLastname, cust.getLastName());
    }
    
    @Test
    public void retriveShouldReturnTheCustomer() {
      Optional<Customer> optionalCustomer = dao.getById(CUSTOMER.getId());
      
      assertTrue(optionalCustomer.isPresent());
      assertEquals(CUSTOMER, optionalCustomer.get());
    }
  }

  
  private int getNonExistingCustomerId() {
    return 999;
  }
  
  private void assertCustomerCountIs(int count) throws Exception {
    try (Stream<Customer> allCustomers = dao.getAll()) {
      assertTrue(allCustomers.count() == count);
    }
  }
}
