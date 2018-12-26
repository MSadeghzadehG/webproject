

package org.springframework.boot.autoconfigure.couchbase;

import com.couchbase.client.java.Bucket;
import com.couchbase.client.java.Cluster;
import com.couchbase.client.java.CouchbaseBucket;
import com.couchbase.client.java.cluster.ClusterInfo;
import com.couchbase.client.java.env.CouchbaseEnvironment;
import org.junit.Rule;
import org.junit.Test;

import org.springframework.boot.autoconfigure.AutoConfigurations;
import org.springframework.boot.autoconfigure.context.PropertyPlaceholderAutoConfiguration;
import org.springframework.boot.test.context.runner.ApplicationContextRunner;
import org.springframework.context.annotation.Bean;
import org.springframework.context.annotation.Configuration;

import static org.assertj.core.api.Assertions.assertThat;
import static org.mockito.Mockito.mock;


public class CouchbaseAutoConfigurationIntegrationTests {

	private ApplicationContextRunner contextRunner = new ApplicationContextRunner()
			.withConfiguration(
					AutoConfigurations.of(PropertyPlaceholderAutoConfiguration.class,
							CouchbaseAutoConfiguration.class));

	@Rule
	public final CouchbaseTestServer couchbase = new CouchbaseTestServer();

	@Test
	public void defaultConfiguration() {
		this.contextRunner.withPropertyValues("spring.couchbase.bootstrapHosts=localhost")
				.run((context) -> assertThat(context).hasSingleBean(Cluster.class)
						.hasSingleBean(ClusterInfo.class)
						.hasSingleBean(CouchbaseEnvironment.class)
						.hasSingleBean(Bucket.class));
	}

	@Test
	public void customConfiguration() {
		this.contextRunner.withUserConfiguration(CustomConfiguration.class)
				.withPropertyValues("spring.couchbase.bootstrapHosts=localhost")
				.run((context) -> {
					assertThat(context.getBeansOfType(Cluster.class)).hasSize(2);
					assertThat(context.getBeansOfType(ClusterInfo.class)).hasSize(1);
					assertThat(context.getBeansOfType(CouchbaseEnvironment.class))
							.hasSize(1);
					assertThat(context.getBeansOfType(Bucket.class)).hasSize(2);
				});
	}

	@Configuration
	static class CustomConfiguration {

		@Bean
		public Cluster myCustomCouchbaseCluster() {
			return mock(Cluster.class);
		}

		@Bean
		public Bucket myCustomCouchbaseClient() {
			return mock(CouchbaseBucket.class);
		}

	}

}
