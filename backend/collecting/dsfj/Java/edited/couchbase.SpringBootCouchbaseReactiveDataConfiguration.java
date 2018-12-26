

package org.springframework.boot.autoconfigure.data.couchbase;

import org.springframework.boot.autoconfigure.condition.ConditionalOnBean;
import org.springframework.boot.autoconfigure.condition.ConditionalOnMissingBean;
import org.springframework.context.annotation.Bean;
import org.springframework.context.annotation.Configuration;
import org.springframework.data.couchbase.config.AbstractReactiveCouchbaseDataConfiguration;
import org.springframework.data.couchbase.config.BeanNames;
import org.springframework.data.couchbase.config.CouchbaseConfigurer;
import org.springframework.data.couchbase.core.RxJavaCouchbaseTemplate;
import org.springframework.data.couchbase.core.query.Consistency;
import org.springframework.data.couchbase.repository.config.ReactiveRepositoryOperationsMapping;


@Configuration
@ConditionalOnMissingBean(AbstractReactiveCouchbaseDataConfiguration.class)
@ConditionalOnBean(CouchbaseConfigurer.class)
class SpringBootCouchbaseReactiveDataConfiguration
		extends AbstractReactiveCouchbaseDataConfiguration {

	private final CouchbaseDataProperties properties;

	private final CouchbaseConfigurer couchbaseConfigurer;

	SpringBootCouchbaseReactiveDataConfiguration(CouchbaseDataProperties properties,
			CouchbaseConfigurer couchbaseConfigurer) {
		this.properties = properties;
		this.couchbaseConfigurer = couchbaseConfigurer;
	}

	@Override
	protected CouchbaseConfigurer couchbaseConfigurer() {
		return this.couchbaseConfigurer;
	}

	@Override
	protected Consistency getDefaultConsistency() {
		return this.properties.getConsistency();
	}

	@Override
	@ConditionalOnMissingBean(name = BeanNames.RXJAVA1_COUCHBASE_TEMPLATE)
	@Bean(name = BeanNames.RXJAVA1_COUCHBASE_TEMPLATE)
	public RxJavaCouchbaseTemplate reactiveCouchbaseTemplate() throws Exception {
		return super.reactiveCouchbaseTemplate();
	}

	@Override
	@ConditionalOnMissingBean(name = BeanNames.REACTIVE_COUCHBASE_OPERATIONS_MAPPING)
	@Bean(name = BeanNames.REACTIVE_COUCHBASE_OPERATIONS_MAPPING)
	public ReactiveRepositoryOperationsMapping reactiveRepositoryOperationsMapping(
			RxJavaCouchbaseTemplate reactiveCouchbaseTemplate) throws Exception {
		return super.reactiveRepositoryOperationsMapping(reactiveCouchbaseTemplate);
	}

}
