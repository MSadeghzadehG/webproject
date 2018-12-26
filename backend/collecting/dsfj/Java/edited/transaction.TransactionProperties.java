

package org.springframework.boot.autoconfigure.transaction;

import java.time.Duration;
import java.time.temporal.ChronoUnit;

import org.springframework.boot.context.properties.ConfigurationProperties;
import org.springframework.boot.convert.DurationUnit;
import org.springframework.transaction.support.AbstractPlatformTransactionManager;


@ConfigurationProperties(prefix = "spring.transaction")
public class TransactionProperties implements
		PlatformTransactionManagerCustomizer<AbstractPlatformTransactionManager> {

	
	@DurationUnit(ChronoUnit.SECONDS)
	private Duration defaultTimeout;

	
	private Boolean rollbackOnCommitFailure;

	public Duration getDefaultTimeout() {
		return this.defaultTimeout;
	}

	public void setDefaultTimeout(Duration defaultTimeout) {
		this.defaultTimeout = defaultTimeout;
	}

	public Boolean getRollbackOnCommitFailure() {
		return this.rollbackOnCommitFailure;
	}

	public void setRollbackOnCommitFailure(Boolean rollbackOnCommitFailure) {
		this.rollbackOnCommitFailure = rollbackOnCommitFailure;
	}

	@Override
	public void customize(AbstractPlatformTransactionManager transactionManager) {
		if (this.defaultTimeout != null) {
			transactionManager.setDefaultTimeout((int) this.defaultTimeout.getSeconds());
		}
		if (this.rollbackOnCommitFailure != null) {
			transactionManager.setRollbackOnCommitFailure(this.rollbackOnCommitFailure);
		}
	}

}
