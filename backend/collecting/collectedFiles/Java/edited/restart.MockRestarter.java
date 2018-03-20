

package org.springframework.boot.devtools.restart;

import java.net.URL;
import java.util.HashMap;
import java.util.Map;

import org.junit.rules.TestRule;
import org.junit.runner.Description;
import org.junit.runners.model.Statement;

import org.springframework.beans.factory.ObjectFactory;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyString;
import static org.mockito.BDDMockito.given;
import static org.mockito.Mockito.mock;


public class MockRestarter implements TestRule {

	private Map<String, Object> attributes = new HashMap<>();

	private Restarter mock = mock(Restarter.class);

	@Override
	public Statement apply(Statement base, Description description) {
		return new Statement() {

			@Override
			public void evaluate() throws Throwable {
				setup();
				base.evaluate();
				cleanup();
			}

		};
	}

	@SuppressWarnings("rawtypes")
	private void setup() {
		Restarter.setInstance(this.mock);
		given(this.mock.getInitialUrls()).willReturn(new URL[] {});
		given(this.mock.getOrAddAttribute(anyString(), any(ObjectFactory.class)))
				.willAnswer((invocation) -> {
					String name = invocation.getArgument(0);
					ObjectFactory factory = invocation.getArgument(1);
					Object attribute = MockRestarter.this.attributes.get(name);
					if (attribute == null) {
						attribute = factory.getObject();
						MockRestarter.this.attributes.put(name, attribute);
					}
					return attribute;
				});
		given(this.mock.getThreadFactory()).willReturn(Thread::new);
	}

	private void cleanup() {
		this.attributes.clear();
		Restarter.clearInstance();
	}

	public Restarter getMock() {
		return this.mock;
	}

}
