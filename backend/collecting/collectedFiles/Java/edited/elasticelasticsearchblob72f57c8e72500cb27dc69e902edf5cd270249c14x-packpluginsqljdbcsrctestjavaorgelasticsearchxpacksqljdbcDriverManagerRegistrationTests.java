
package org.elasticsearch.xpack.sql.jdbc;

import org.elasticsearch.Version;
import org.elasticsearch.test.ESTestCase;
import org.elasticsearch.xpack.sql.jdbc.jdbc.JdbcDriver;

import java.security.AccessController;
import java.security.PrivilegedExceptionAction;
import java.sql.Driver;
import java.sql.DriverManager;
import java.sql.SQLException;
import java.util.function.Consumer;

public class DriverManagerRegistrationTests extends ESTestCase {

    public void testRegistration() throws Exception {
        driverManagerTemplate(d -> assertNotNull(d));
    }

    public void testVersioning() throws Exception {
        driverManagerTemplate(d -> {
            

            assertNotEquals(String.valueOf(Version.CURRENT.major), d.getMajorVersion());
            assertNotEquals(String.valueOf(Version.CURRENT.minor), d.getMinorVersion());
        });
    }

    private static void driverManagerTemplate(Consumer<JdbcDriver> c) throws Exception {
        String url = "jdbc:es:localhost:9200/";
        Driver driver = null;
        try {
                        driver = DriverManager.getDriver(url);
        } catch (SQLException ex) {
            assertEquals("No suitable driver", ex.getMessage());
        }
        boolean set = driver != null;

        try {
            JdbcDriver d = JdbcDriver.register();
            if (driver != null) {
                assertEquals(driver, d);
            }

            c.accept(d);

            AccessController.doPrivileged((PrivilegedExceptionAction<Void>) () -> {
                                JdbcDriver.deregister();
                return null;
            });

            SQLException ex = expectThrows(SQLException.class, () -> DriverManager.getDriver(url));
            assertEquals("No suitable driver", ex.getMessage());
        } finally {
            if (set) {
                JdbcDriver.register();
            }
        }
    }
}
