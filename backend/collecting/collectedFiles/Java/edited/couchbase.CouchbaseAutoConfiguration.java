

package org.springframework.boot.autoconfigure.couchbase;

import com.couchbase.client.java.Bucket;
import com.couchbase.client.java.Cluster;
import com.couchbase.client.java.CouchbaseBucket;
import com.couchbase.client.java.CouchbaseCluster;
import com.couchbase.client.java.cluster.ClusterInfo;
import com.couchbase.client.java.env.DefaultCouchbaseEnvironment;

import org.springframework.boot.autoconfigure.EnableAutoConfiguration;
import org.springframework.boot.autoconfigure.condition.AnyNestedCondition;
import org.springframework.boot.autoconfigure.condition.ConditionalOnBean;
import org.springframework.boot.autoconfigure.condition.ConditionalOnClass;
import org.springframework.boot.autoconfigure.condition.ConditionalOnMissingBean;
import org.springframework.boot.context.properties.EnableConfigurationProperties;
import org.springframework.context.annotation.Bean;
import org.springframework.context.annotation.Conditional;
import org.springframework.context.annotation.Configuration;
import org.springframework.context.annotation.DependsOn;
import org.springframework.context.annotation.Primary;


@Configuration
@ConditionalOnClass({ CouchbaseBucket.class, Cluster.class })
@Conditional(CouchbaseAutoConfiguration.CouchbaseCondition.class)
@EnableConfigurationProperties(CouchbaseProperties.class)
public class CouchbaseAutoConfiguration {

	@Configuration
	@ConditionalOnMissingBean(value = CouchbaseConfiguration.class, type = "org.springframework.data.couchbase.config.CouchbaseConfigurer")
	public static class CouchbaseConfiguration {

		private final CouchbaseProperties properties;

		public CouchbaseConfiguration(CouchbaseProperties properties) {
			this.properties = properties;
		}

		@Bean
		@Primary
		public DefaultCouchbaseEnvironment couchbaseEnvironment() throws Exception {
			return initializeEnvironmentBuilder(this.properties).build();
		}

		@Bean
		@Primary
		public Cluster couchbaseCluster() throws Exception {
			return CouchbaseCluster.create(couchbaseEnvironment(),
					this.properties.getBootstrapHosts());
		}

		@Bean
		@Primary
		@DependsOn("couchbaseClient")
		public ClusterInfo couchbaseClusterInfo() throws Exception {
			return couchbaseCluster()
					.clusterManager(this.properties.getBucket().getName(),
							this.properties.getBucket().getPassword())
					.info();
		}

		@Bean
		@Primary
		public Bucket couchbaseClient() throws Exception {
			return couchbaseCluster().openBucket(this.properties.getBucket().getName(),
					this.properties.getBucket().getPassword());
		}

		
		protected DefaultCouchbaseEnvironment.Builder initializeEnvironmentBuilder(
				CouchbaseProperties properties) {
			CouchbaseProperties.Endpoints endpoints = properties.getEnv().getEndpoints();
			CouchbaseProperties.Timeouts timeouts = properties.getEnv().getTimeouts();
			DefaultCouchbaseEnvironment.Builder builder = DefaultCouchbaseEnvironment
					.builder();
			if (timeouts.getConnect() != null) {
				builder = builder.connectTimeout(timeouts.getConnect().toMillis());
			}
			builder = builder.kvEndpoints(endpoints.getKeyValue());
			if (timeouts.getKeyValue() != null) {
				builder = builder.kvTimeout(timeouts.getKeyValue().toMillis());
			}
			builder = builder.queryEndpoints(endpoints.getQuery());
			if (timeouts.getQuery() != null) {
				builder = builder.queryTimeout(timeouts.getQuery().toMillis())
						.viewEndpoints(endpoints.getView());
			}
			if (timeouts.getSocketConnect() != null) {
				builder = builder.socketConnectTimeout(
						(int) timeouts.getSocketConnect().toMillis());
			}
			if (timeouts.getView() != null) {
				builder = builder.viewTimeout(timeouts.getView().toMillis());
			}
			CouchbaseProperties.Ssl ssl = properties.getEnv().getSsl();
			if (ssl.getEnabled()) {
				builder.sslEnabled(true);
				if (ssl.getKeyStore() != null) {
					builder.sslKeystoreFile(ssl.getKeyStore());
				}
				if (ssl.getKeyStorePassword() != null) {
					builder.sslKeystorePassword(ssl.getKeyStorePassword());
				}
			}
			return builder;
		}

	}

	
	static class CouchbaseCondition extends AnyNestedCondition {

		CouchbaseCondition() {
			super(ConfigurationPhase.REGISTER_BEAN);
		}

		@Conditional(OnBootstrapHostsCondition.class)
		static class BootstrapHostsProperty {

		}

		@ConditionalOnBean(type = "org.springframework.data.couchbase.config.CouchbaseConfigurer")
		static class CouchbaseConfigurerAvailable {

		}

	}

}
