

package org.springframework.boot.autoconfigure.transaction;

import java.util.ArrayList;
import java.util.List;

import org.junit.Test;

import org.springframework.transaction.PlatformTransactionManager;
import org.springframework.transaction.jta.JtaTransactionManager;

import static org.assertj.core.api.Assertions.assertThat;
import static org.mockito.Mockito.mock;


public class TransactionManagerCustomizersTests {

	@Test
	public void customizeWithNullCustomizersShouldDoNothing() {
		new TransactionManagerCustomizers(null)
				.customize(mock(PlatformTransactionManager.class));
	}

	@Test
	public void customizeShouldCheckGeneric() {
		List<TestCustomizer<?>> list = new ArrayList<>();
		list.add(new TestCustomizer<>());
		list.add(new TestJtaCustomizer());
		TransactionManagerCustomizers customizers = new TransactionManagerCustomizers(
				list);
		customizers.customize(mock(PlatformTransactionManager.class));
		customizers.customize(mock(JtaTransactionManager.class));
		assertThat(list.get(0).getCount()).isEqualTo(2);
		assertThat(list.get(1).getCount()).isEqualTo(1);
	}

	private static class TestCustomizer<T extends PlatformTransactionManager>
			implements PlatformTransactionManagerCustomizer<T> {

		private int count;

		@Override
		public void customize(T transactionManager) {
			this.count++;
		}

		public int getCount() {
			return this.count;
		}

	}

	private static class TestJtaCustomizer extends TestCustomizer<JtaTransactionManager> {

	}

}
