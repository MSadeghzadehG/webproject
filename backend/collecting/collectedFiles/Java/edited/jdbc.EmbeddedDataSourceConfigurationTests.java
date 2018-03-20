

package org.springframework.boot.autoconfigure.jdbc;

import java.sql.Connection;
import java.sql.ResultSet;
import java.sql.SQLException;

import javax.sql.DataSource;

import org.junit.After;
import org.junit.Test;

import org.springframework.boot.test.util.TestPropertyValues;
import org.springframework.context.annotation.AnnotationConfigApplicationContext;

import static org.assertj.core.api.Assertions.assertThat;


public class EmbeddedDataSourceConfigurationTests {

	private AnnotationConfigApplicationContext context;

	@After
	public void closeContext() {
		if (this.context != null) {
			this.context.close();
		}
	}

	@Test
	public void defaultEmbeddedDatabase() {
		this.context = load();
		assertThat(this.context.getBean(DataSource.class)).isNotNull();
	}

	@Test
	public void generateUniqueName() throws Exception {
		this.context = load("spring.datasource.generate-unique-name=true");
		try (AnnotationConfigApplicationContext context2 = load(
				"spring.datasource.generate-unique-name=true")) {
			DataSource dataSource = this.context.getBean(DataSource.class);
			DataSource dataSource2 = context2.getBean(DataSource.class);
			assertThat(getDatabaseName(dataSource))
					.isNotEqualTo(getDatabaseName(dataSource2));
		}
	}

	private String getDatabaseName(DataSource dataSource) throws SQLException {
		try (Connection connection = dataSource.getConnection()) {
			ResultSet catalogs = connection.getMetaData().getCatalogs();
			if (catalogs.next()) {
				return catalogs.getString(1);
			}
			else {
				throw new IllegalStateException("Unable to get database name");
			}
		}
	}

	private AnnotationConfigApplicationContext load(String... environment) {
		AnnotationConfigApplicationContext ctx = new AnnotationConfigApplicationContext();
		TestPropertyValues.of(environment).applyTo(ctx);
		ctx.register(EmbeddedDataSourceConfiguration.class);
		ctx.refresh();
		return ctx;
	}

}
