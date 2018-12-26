

package org.springframework.boot.actuate.jdbc;

import java.sql.Connection;
import java.sql.ResultSet;
import java.sql.ResultSetMetaData;
import java.sql.SQLException;
import java.util.List;

import javax.sql.DataSource;

import org.springframework.beans.factory.InitializingBean;
import org.springframework.boot.actuate.health.AbstractHealthIndicator;
import org.springframework.boot.actuate.health.Health;
import org.springframework.boot.actuate.health.HealthIndicator;
import org.springframework.boot.jdbc.DatabaseDriver;
import org.springframework.dao.support.DataAccessUtils;
import org.springframework.jdbc.IncorrectResultSetColumnCountException;
import org.springframework.jdbc.core.ConnectionCallback;
import org.springframework.jdbc.core.JdbcTemplate;
import org.springframework.jdbc.core.RowMapper;
import org.springframework.jdbc.support.JdbcUtils;
import org.springframework.util.Assert;
import org.springframework.util.StringUtils;


public class DataSourceHealthIndicator extends AbstractHealthIndicator
		implements InitializingBean {

	private static final String DEFAULT_QUERY = "SELECT 1";

	private DataSource dataSource;

	private String query;

	private JdbcTemplate jdbcTemplate;

	
	public DataSourceHealthIndicator() {
		this(null, null);
	}

	
	public DataSourceHealthIndicator(DataSource dataSource) {
		this(dataSource, null);
	}

	
	public DataSourceHealthIndicator(DataSource dataSource, String query) {
		super("DataSource health check failed");
		this.dataSource = dataSource;
		this.query = query;
		this.jdbcTemplate = (dataSource == null ? null : new JdbcTemplate(dataSource));
	}

	@Override
	public void afterPropertiesSet() throws Exception {
		Assert.state(this.dataSource != null,
				"DataSource for DataSourceHealthIndicator must be specified");
	}

	@Override
	protected void doHealthCheck(Health.Builder builder) throws Exception {
		if (this.dataSource == null) {
			builder.up().withDetail("database", "unknown");
		}
		else {
			doDataSourceHealthCheck(builder);
		}
	}

	private void doDataSourceHealthCheck(Health.Builder builder) throws Exception {
		String product = getProduct();
		builder.up().withDetail("database", product);
		String validationQuery = getValidationQuery(product);
		if (StringUtils.hasText(validationQuery)) {
						List<Object> results = this.jdbcTemplate.query(validationQuery,
					new SingleColumnRowMapper());
			Object result = DataAccessUtils.requiredSingleResult(results);
			builder.withDetail("hello", result);
		}
	}

	private String getProduct() {
		return this.jdbcTemplate.execute((ConnectionCallback<String>) this::getProduct);
	}

	private String getProduct(Connection connection) throws SQLException {
		return connection.getMetaData().getDatabaseProductName();
	}

	protected String getValidationQuery(String product) {
		String query = this.query;
		if (!StringUtils.hasText(query)) {
			DatabaseDriver specific = DatabaseDriver.fromProductName(product);
			query = specific.getValidationQuery();
		}
		if (!StringUtils.hasText(query)) {
			query = DEFAULT_QUERY;
		}
		return query;
	}

	
	public void setDataSource(DataSource dataSource) {
		this.dataSource = dataSource;
		this.jdbcTemplate = new JdbcTemplate(dataSource);
	}

	
	public void setQuery(String query) {
		this.query = query;
	}

	
	public String getQuery() {
		return this.query;
	}

	
	private static class SingleColumnRowMapper implements RowMapper<Object> {

		@Override
		public Object mapRow(ResultSet rs, int rowNum) throws SQLException {
			ResultSetMetaData metaData = rs.getMetaData();
			int columns = metaData.getColumnCount();
			if (columns != 1) {
				throw new IncorrectResultSetColumnCountException(1, columns);
			}
			return JdbcUtils.getResultSetValue(rs, 1);
		}

	}

}
