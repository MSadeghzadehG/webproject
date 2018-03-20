

package org.springframework.boot.devtools.autoconfigure;

import java.sql.SQLException;
import java.sql.Statement;

import javax.sql.DataSource;

import org.junit.Test;
import org.junit.runner.RunWith;

import org.springframework.boot.autoconfigure.jdbc.DataSourceAutoConfiguration;
import org.springframework.boot.testsupport.runner.classpath.ClassPathExclusions;
import org.springframework.boot.testsupport.runner.classpath.ModifiedClassPathRunner;
import org.springframework.context.ConfigurableApplicationContext;

import static org.mockito.Mockito.never;
import static org.mockito.Mockito.verify;


@RunWith(ModifiedClassPathRunner.class)
@ClassPathExclusions("HikariCP-*.jar")
public class DevToolsEmbeddedDataSourceAutoConfigurationTests
		extends AbstractDevToolsDataSourceAutoConfigurationTests {

	@Test
	public void autoConfiguredDataSourceIsNotShutdown() throws SQLException {
		ConfigurableApplicationContext context = createContext(
				DataSourceAutoConfiguration.class, DataSourceSpyConfiguration.class);
		Statement statement = configureDataSourceBehavior(
				context.getBean(DataSource.class));
		context.close();
		verify(statement, never()).execute("SHUTDOWN");
	}

}
