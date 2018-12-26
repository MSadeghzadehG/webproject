

package org.springframework.boot.autoconfigure.quartz;

import java.util.HashMap;
import java.util.Map;

import org.springframework.boot.context.properties.ConfigurationProperties;
import org.springframework.boot.jdbc.DataSourceInitializationMode;


@ConfigurationProperties("spring.quartz")
public class QuartzProperties {

	
	private JobStoreType jobStoreType = JobStoreType.MEMORY;

	
	private final Map<String, String> properties = new HashMap<>();

	private final Jdbc jdbc = new Jdbc();

	public JobStoreType getJobStoreType() {
		return this.jobStoreType;
	}

	public void setJobStoreType(JobStoreType jobStoreType) {
		this.jobStoreType = jobStoreType;
	}

	public Map<String, String> getProperties() {
		return this.properties;
	}

	public Jdbc getJdbc() {
		return this.jdbc;
	}

	public static class Jdbc {

		private static final String DEFAULT_SCHEMA_LOCATION = "classpath:org/quartz/impl/"
				+ "jdbcjobstore/tables_@@platform@@.sql";

		
		private String schema = DEFAULT_SCHEMA_LOCATION;

		
		private DataSourceInitializationMode initializeSchema = DataSourceInitializationMode.EMBEDDED;

		public String getSchema() {
			return this.schema;
		}

		public void setSchema(String schema) {
			this.schema = schema;
		}

		public DataSourceInitializationMode getInitializeSchema() {
			return this.initializeSchema;
		}

		public void setInitializeSchema(DataSourceInitializationMode initializeSchema) {
			this.initializeSchema = initializeSchema;
		}

	}

}
