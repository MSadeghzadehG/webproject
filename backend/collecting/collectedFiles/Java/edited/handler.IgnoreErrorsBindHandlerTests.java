

package org.springframework.boot.context.properties.bind.handler;

import java.util.ArrayList;
import java.util.List;

import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.ExpectedException;

import org.springframework.boot.context.properties.bind.BindException;
import org.springframework.boot.context.properties.bind.Bindable;
import org.springframework.boot.context.properties.bind.Binder;
import org.springframework.boot.context.properties.source.ConfigurationPropertySource;
import org.springframework.boot.context.properties.source.MockConfigurationPropertySource;

import static org.assertj.core.api.Assertions.assertThat;


public class IgnoreErrorsBindHandlerTests {

	@Rule
	public ExpectedException thrown = ExpectedException.none();

	private List<ConfigurationPropertySource> sources = new ArrayList<>();

	private Binder binder;

	@Before
	public void setup() {
		MockConfigurationPropertySource source = new MockConfigurationPropertySource();
		source.put("example.foo", "bar");
		this.sources.add(source);
		this.binder = new Binder(this.sources);
	}

	@Test
	public void bindWhenNotIgnoringErrorsShouldFail() {
		this.thrown.expect(BindException.class);
		this.binder.bind("example", Bindable.of(Example.class));
	}

	@Test
	public void bindWhenIgnoringErrorsShouldBind() {
		Example bound = this.binder.bind("example", Bindable.of(Example.class),
				new IgnoreErrorsBindHandler()).get();
		assertThat(bound.getFoo()).isEqualTo(0);
	}

	public static class Example {

		private int foo;

		public int getFoo() {
			return this.foo;
		}

		public void setFoo(int foo) {
			this.foo = foo;
		}

	}

}
