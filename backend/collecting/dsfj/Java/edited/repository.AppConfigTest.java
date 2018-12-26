
package com.iluwatar.repository;

import static org.junit.jupiter.api.Assertions.assertNotNull;
import static org.junit.jupiter.api.Assertions.assertTrue;

import java.sql.ResultSet;
import java.sql.SQLException;

import javax.sql.DataSource;

import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.extension.ExtendWith;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.test.context.ContextConfiguration;
import org.springframework.test.context.junit.jupiter.SpringExtension;
import org.springframework.test.context.support.AnnotationConfigContextLoader;
import org.springframework.transaction.annotation.Transactional;


@ExtendWith(SpringExtension.class)
@ContextConfiguration(classes = { AppConfig.class }, loader = AnnotationConfigContextLoader.class)
public class AppConfigTest {

  @Autowired
  DataSource dataSource;

  
  @Test
  public void testDataSource() {
    assertNotNull(dataSource);
  }

  
  @Test
  @Transactional
  public void testQuery() throws SQLException {
    ResultSet resultSet = dataSource.getConnection().createStatement().executeQuery("SELECT 1");
    String result = null;
    String expected = "1";
    while (resultSet.next()) {
      result = resultSet.getString(1);

    }
    assertTrue(result.equals(expected));
  }

}
