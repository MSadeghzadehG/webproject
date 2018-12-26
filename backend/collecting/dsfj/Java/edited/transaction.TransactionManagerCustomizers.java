

package org.springframework.boot.autoconfigure.transaction;

import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.List;

import org.springframework.boot.util.LambdaSafe;
import org.springframework.transaction.PlatformTransactionManager;


public class TransactionManagerCustomizers {

	private final List<PlatformTransactionManagerCustomizer<?>> customizers;

	public TransactionManagerCustomizers(
			Collection<? extends PlatformTransactionManagerCustomizer<?>> customizers) {
		this.customizers = (customizers == null ? Collections.emptyList()
				: new ArrayList<>(customizers));
	}

	@SuppressWarnings("unchecked")
	public void customize(PlatformTransactionManager transactionManager) {
		LambdaSafe
				.callbacks(PlatformTransactionManagerCustomizer.class, this.customizers,
						transactionManager)
				.withLogger(TransactionManagerCustomizers.class)
				.invoke((customizer) -> customizer.customize(transactionManager));
	}

}
