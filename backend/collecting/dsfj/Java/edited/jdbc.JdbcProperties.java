

package org.springframework.boot.autoconfigure.jdbc;

import java.time.Duration;
import java.time.temporal.ChronoUnit;

import org.springframework.boot.context.properties.ConfigurationProperties;
import org.springframework.boot.convert.DurationUnit;


@ConfigurationProperties(prefix = "spring.jdbc")
public class JdbcProperties {

	private final Template template = new Template();

	public Template getTemplate() {
		return this.template;
	}

	
	public static class Template {

		
		private int fetchSize = -1;

		
		private int maxRows = -1;

		
		@DurationUnit(ChronoUnit.SECONDS)
		private Duration queryTimeout;

		public int getFetchSize() {
			return this.fetchSize;
		}

		public void setFetchSize(int fetchSize) {
			this.fetchSize = fetchSize;
		}

		public int getMaxRows() {
			return this.maxRows;
		}

		public void setMaxRows(int maxRows) {
			this.maxRows = maxRows;
		}

		public Duration getQueryTimeout() {
			return this.queryTimeout;
		}

		public void setQueryTimeout(Duration queryTimeout) {
			this.queryTimeout = queryTimeout;
		}

	}

}
