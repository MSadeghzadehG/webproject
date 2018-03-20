

package org.springframework.boot.autoconfigure.jdbc;

import java.sql.Connection;
import java.sql.DatabaseMetaData;
import java.sql.SQLException;
import java.util.UUID;

import javax.sql.DataSource;

import com.zaxxer.hikari.HikariDataSource;
import org.junit.Test;

import org.springframework.boot.jdbc.DataSourceBuilder;
import org.springframework.boot.jdbc.DataSourceInitializationMode;
import org.springframework.jdbc.core.JdbcTemplate;

import static org.assertj.core.api.Assertions.assertThat;
import static org.mockito.BDDMockito.given;
import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.verify;


public class DataSourceInitializerTests {

	@Test
	public void initializeEmbeddedByDefault() {
		try (HikariDataSource dataSource = createDataSource()) {
			DataSourceInitializer initializer = new DataSourceInitializer(dataSource,
					new DataSourceProperties());
			JdbcTemplate jdbcTemplate = new JdbcTemplate(dataSource);
			assertThat(initializer.createSchema()).isTrue();
			assertNumberOfRows(jdbcTemplate, 0);
			initializer.initSchema();
			assertNumberOfRows(jdbcTemplate, 1);
		}
	}

	@Test
	public void initializeWithModeAlways() {
		try (HikariDataSource dataSource = createDataSource()) {
			DataSourceProperties properties = new DataSourceProperties();
			properties.setInitializationMode(DataSourceInitializationMode.ALWAYS);
			DataSourceInitializer initializer = new DataSourceInitializer(dataSource,
					properties);
			JdbcTemplate jdbcTemplate = new JdbcTemplate(dataSource);
			assertThat(initializer.createSchema()).isTrue();
			assertNumberOfRows(jdbcTemplate, 0);
			initializer.initSchema();
			assertNumberOfRows(jdbcTemplate, 1);
		}
	}

	private void assertNumberOfRows(JdbcTemplate jdbcTemplate, int count) {
		assertThat(jdbcTemplate.queryForObject("SELECT COUNT(*) from BAR", Integer.class))
				.isEqualTo(count);
	}

	@Test
	public void initializeWithModeNever() {
		try (HikariDataSource dataSource = createDataSource()) {
			DataSourceProperties properties = new DataSourceProperties();
			properties.setInitializationMode(DataSourceInitializationMode.NEVER);
			DataSourceInitializer initializer = new DataSourceInitializer(dataSource,
					properties);
			assertThat(initializer.createSchema()).isFalse();
		}
	}

	@Test
	public void initializeOnlyEmbeddedByDefault() throws SQLException {
		DatabaseMetaData metadata = mock(DatabaseMetaData.class);
		given(metadata.getDatabaseProductName()).willReturn("MySQL");
		Connection connection = mock(Connection.class);
		given(connection.getMetaData()).willReturn(metadata);
		DataSource dataSource = mock(DataSource.class);
		given(dataSource.getConnection()).willReturn(connection);
		DataSourceInitializer initializer = new DataSourceInitializer(dataSource,
				new DataSourceProperties());
		assertThat(initializer.createSchema()).isFalse();
		verify(dataSource).getConnection();
	}

	private HikariDataSource createDataSource() {
		return DataSourceBuilder.create().type(HikariDataSource.class)
				.url("jdbc:h2:mem:" + UUID.randomUUID()).build();
	}

}
