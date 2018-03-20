

package org.springframework.boot.autoconfigure.transaction.jta;

import org.springframework.beans.factory.ObjectProvider;
import org.springframework.boot.autoconfigure.condition.ConditionalOnClass;
import org.springframework.boot.autoconfigure.condition.ConditionalOnJndi;
import org.springframework.boot.autoconfigure.condition.ConditionalOnMissingBean;
import org.springframework.boot.autoconfigure.transaction.TransactionManagerCustomizers;
import org.springframework.context.annotation.Bean;
import org.springframework.context.annotation.Configuration;
import org.springframework.transaction.PlatformTransactionManager;
import org.springframework.transaction.config.JtaTransactionManagerFactoryBean;
import org.springframework.transaction.jta.JtaTransactionManager;


@Configuration
@ConditionalOnClass(JtaTransactionManager.class)
@ConditionalOnJndi({ JtaTransactionManager.DEFAULT_USER_TRANSACTION_NAME,
		"java:comp/TransactionManager", "java:appserver/TransactionManager",
		"java:pm/TransactionManager", "java:/TransactionManager" })
@ConditionalOnMissingBean(PlatformTransactionManager.class)
class JndiJtaConfiguration {

	private final TransactionManagerCustomizers transactionManagerCustomizers;

	JndiJtaConfiguration(
			ObjectProvider<TransactionManagerCustomizers> transactionManagerCustomizers) {
		this.transactionManagerCustomizers = transactionManagerCustomizers
				.getIfAvailable();
	}

	@Bean
	public JtaTransactionManager transactionManager() {
		JtaTransactionManager jtaTransactionManager = new JtaTransactionManagerFactoryBean()
				.getObject();
		if (this.transactionManagerCustomizers != null) {
			this.transactionManagerCustomizers.customize(jtaTransactionManager);
		}
		return jtaTransactionManager;
	}

}
