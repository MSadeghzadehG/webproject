

package com.iluwatar.datatransfer;

import org.junit.jupiter.api.Test;

import java.util.ArrayList;
import java.util.List;

import static org.junit.jupiter.api.Assertions.assertEquals;


public class CustomerResourceTest {
  @Test
  public void shouldGetAllCustomers() {
    CustomerDto customer = new CustomerDto("1", "Melody", "Yates");
    List<CustomerDto> customers = new ArrayList<>();
    customers.add(customer);

    CustomerResource customerResource = new CustomerResource(customers);

    List<CustomerDto> allCustomers = customerResource.getAllCustomers();

    assertEquals(allCustomers.size(), 1);
    assertEquals(allCustomers.get(0).getId(), "1");
    assertEquals(allCustomers.get(0).getFirstName(), "Melody");
    assertEquals(allCustomers.get(0).getLastName(), "Yates");
  }

  @Test
  public void shouldSaveCustomer() {
    CustomerDto customer = new CustomerDto("1", "Rita", "Reynolds");
    CustomerResource customerResource = new CustomerResource(new ArrayList<>());

    customerResource.save(customer);

    List<CustomerDto> allCustomers = customerResource.getAllCustomers();
    assertEquals(allCustomers.get(0).getId(), "1");
    assertEquals(allCustomers.get(0).getFirstName(), "Rita");
    assertEquals(allCustomers.get(0).getLastName(), "Reynolds");
  }

  @Test
  public void shouldDeleteCustomer() {
    CustomerDto customer = new CustomerDto("1", "Terry", "Nguyen");
    List<CustomerDto> customers = new ArrayList<>();
    customers.add(customer);

    CustomerResource customerResource = new CustomerResource(customers);

    customerResource.delete(customer.getId());

    List<CustomerDto> allCustomers = customerResource.getAllCustomers();
    assertEquals(allCustomers.size(), 0);
  }

}