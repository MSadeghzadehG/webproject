

package org.springframework.boot.actuate.autoconfigure.jdbc;

import java.util.Collection;
import java.util.LinkedHashMap;
import java.util.Map;

import javax.sql.DataSource;

import org.springframework.beans.factory.InitializingBean;
import org.springframework.beans.factory.ObjectProvider;
import org.springframework.boot.actuate.autoconfigure.health.CompositeHealthIndicatorConfiguration;
import org.springframework.boot.actuate.autoconfigure.health.ConditionalOnEnabledHealthIndicator;
import org.springframework.boot.actuate.autoconfigure.health.HealthIndicatorAutoConfiguration;
import org.springframework.boot.actuate.health.HealthIndicator;
import org.springframework.boot.actuate.jdbc.DataSourceHealthIndicator;
import org.springframework.boot.autoconfigure.AutoConfigureAfter;
import org.springframework.boot.autoconfigure.AutoConfigureBefore;
import org.springframework.boot.autoconfigure.EnableAutoConfiguration;
import org.springframework.boot.autoconfigure.condition.ConditionalOnBean;
import org.springframework.boot.autoconfigure.condition.ConditionalOnClass;
import org.springframework.boot.autoconfigure.condition.ConditionalOnMissingBean;
import org.springframework.boot.autoconfigure.jdbc.DataSourceAutoConfiguration;
import org.springframework.boot.jdbc.metadata.CompositeDataSourcePoolMetadataProvider;
import org.springframework.boot.jdbc.metadata.DataSourcePoolMetadata;
import org.springframework.boot.jdbc.metadata.DataSourcePoolMetadataProvider;
import org.springframework.context.annotation.Bean;
import org.springframework.context.annotation.Configuration;
import org.springframework.jdbc.core.JdbcTemplate;
import org.springframework.jdbc.datasource.lookup.AbstractRoutingDataSource;


@Configuration
@ConditionalOnClass({ JdbcTemplate.class, AbstractRoutingDataSource.class })
@ConditionalOnBean(DataSource.class)
@ConditionalOnEnabledHealthIndicator("db")
@AutoConfigureBefore(HealthIndicatorAutoConfiguration.class)
@AutoConfigureAfter(DataSourceAutoConfiguration.class)
public class DataSourceHealthIndicatorAutoConfiguration extends
		CompositeHealthIndicatorConfiguration<DataSourceHealthIndicator, DataSource>
		implements InitializingBean {

	private final Map<String, DataSource> dataSources;

	private final Collection<DataSourcePoolMetadataProvider> metadataProviders;

	private DataSourcePoolMetadataProvider poolMetadataProvider;

	public DataSourceHealthIndicatorAutoConfiguration(
			ObjectProvider<Map<String, DataSource>> dataSources,
			ObjectProvider<Collection<DataSourcePoolMetadataProvider>> metadataProviders) {
		this.dataSources = filterDataSources(dataSources.getIfAvailable());
		this.metadataProviders = metadataProviders.getIfAvailable();
	}

	private Map<String, DataSource> filterDataSources(
			Map<String, DataSource> candidates) {
		if (candidates == null) {
			return null;
		}
		Map<String, DataSource> dataSources = new LinkedHashMap<>();
		for (Map.Entry<String, DataSource> entry : candidates.entrySet()) {
			if (!(entry.getValue() instanceof AbstractRoutingDataSource)) {
				dataSources.put(entry.getKey(), entry.getValue());
			}
		}
		return dataSources;
	}

	@Override
	public void afterPropertiesSet() throws Exception {
		this.poolMetadataProvider = new CompositeDataSourcePoolMetadataProvider(
				this.metadataProviders);
	}

	@Bean
	@ConditionalOnMissingBean(name = "dbHealthIndicator")
	public HealthIndicator dbHealthIndicator() {
		return createHealthIndicator(this.dataSources);
	}

	@Override
	protected DataSourceHealthIndicator createHealthIndicator(DataSource source) {
		return new DataSourceHealthIndicator(source, getValidationQuery(source));
	}

	private String getValidationQuery(DataSource source) {
		DataSourcePoolMetadata poolMetadata = this.poolMetadataProvider
				.getDataSourcePoolMetadata(source);
		return (poolMetadata == null ? null : poolMetadata.getValidationQuery());
	}

}
