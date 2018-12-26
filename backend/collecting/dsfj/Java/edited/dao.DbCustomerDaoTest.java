
package com.iluwatar.dao;

import org.h2.jdbcx.JdbcDataSource;
import org.junit.jupiter.api.AfterEach;
import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.Nested;
import org.junit.jupiter.api.Test;
import org.mockito.Mockito;

import javax.sql.DataSource;
import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.SQLException;
import java.sql.Statement;
import java.util.stream.Stream;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertFalse;
import static org.junit.jupiter.api.Assertions.assertThrows;
import static org.junit.jupiter.api.Assertions.assertTrue;
import static org.junit.jupiter.api.Assumptions.assumeTrue;
import static org.mockito.Mockito.doReturn;
import static org.mockito.Mockito.doThrow;
import static org.mockito.Mockito.mock;


public class DbCustomerDaoTest {

  private static final String DB_URL = "jdbc:h2:~/dao";
  private DbCustomerDao dao;
  private Customer existingCustomer = new Customer(1, "Freddy", "Krueger");

  
  @BeforeEach
  public void createSchema() throws SQLException {
    try (Connection connection = DriverManager.getConnection(DB_URL);
        Statement statement = connection.createStatement()) {
      statement.execute(CustomerSchemaSql.CREATE_SCHEMA_SQL);
    }
  }

  
  @Nested
  public class ConnectionSuccess {

    
    @BeforeEach
    public void setUp() throws Exception {
      JdbcDataSource dataSource = new JdbcDataSource();
      dataSource.setURL(DB_URL);
      dao = new DbCustomerDao(dataSource);
      boolean result = dao.add(existingCustomer);
      assertTrue(result);
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
        Customer existingCustomer = new Customer(1, "Freddy", "Krueger");

        boolean result = dao.add(existingCustomer);

        assertFalse(result);
        assertCustomerCountIs(1);
        assertEquals(existingCustomer, dao.getById(existingCustomer.getId()).get());
      }

      @Test
      public void deletionShouldBeSuccessAndCustomerShouldBeNonAccessible() throws Exception {
        boolean result = dao.delete(existingCustomer);

        assertTrue(result);
        assertCustomerCountIs(0);
        assertFalse(dao.getById(existingCustomer.getId()).isPresent());
      }

      @Test
      public void updationShouldBeSuccessAndAccessingTheSameCustomerShouldReturnUpdatedInformation() throws Exception {
        final String newFirstname = "Bernard";
        final String newLastname = "Montgomery";
        final Customer customer = new Customer(existingCustomer.getId(), newFirstname, newLastname);
        boolean result = dao.update(customer);

        assertTrue(result);

        final Customer cust = dao.getById(existingCustomer.getId()).get();
        assertEquals(newFirstname, cust.getFirstName());
        assertEquals(newLastname, cust.getLastName());
      }
    }
  }

  
  @Nested
  public class ConnectivityIssue {
    
    private static final String EXCEPTION_CAUSE = "Connection not available";

    
    @BeforeEach
    public void setUp() throws SQLException {
      dao = new DbCustomerDao(mockedDatasource());
    }
    
    private DataSource mockedDatasource() throws SQLException {
      DataSource mockedDataSource = mock(DataSource.class);
      Connection mockedConnection = mock(Connection.class);
      SQLException exception = new SQLException(EXCEPTION_CAUSE);
      doThrow(exception).when(mockedConnection).prepareStatement(Mockito.anyString());
      doReturn(mockedConnection).when(mockedDataSource).getConnection();
      return mockedDataSource;
    }

    @Test
    public void addingACustomerFailsWithExceptionAsFeedbackToClient() {
      assertThrows(Exception.class, () -> {
        dao.add(new Customer(2, "Bernard", "Montgomery"));
      });
    }
    
    @Test
    public void deletingACustomerFailsWithExceptionAsFeedbackToTheClient() {
      assertThrows(Exception.class, () -> {
        dao.delete(existingCustomer);
      });
    }
    
    @Test
    public void updatingACustomerFailsWithFeedbackToTheClient() {
      final String newFirstname = "Bernard";
      final String newLastname = "Montgomery";
      assertThrows(Exception.class, () -> {
        dao.update(new Customer(existingCustomer.getId(), newFirstname, newLastname));
      });
    }
    
    @Test
    public void retrievingACustomerByIdFailsWithExceptionAsFeedbackToClient() {
      assertThrows(Exception.class, () -> {
        dao.getById(existingCustomer.getId());
      });
    }
    
    @Test
    public void retrievingAllCustomersFailsWithExceptionAsFeedbackToClient() {
      assertThrows(Exception.class, () -> {
        dao.getAll();
      });
    }

  }

  
  @AfterEach
  public void deleteSchema() throws SQLException {
    try (Connection connection = DriverManager.getConnection(DB_URL);
        Statement statement = connection.createStatement()) {
      statement.execute(CustomerSchemaSql.DELETE_SCHEMA_SQL);
    }
  }

  private void assertCustomerCountIs(int count) throws Exception {
    try (Stream<Customer> allCustomers = dao.getAll()) {
      assertTrue(allCustomers.count() == count);
    }
  }


  
  private int getNonExistingCustomerId() {
    return 999;
  }
}
