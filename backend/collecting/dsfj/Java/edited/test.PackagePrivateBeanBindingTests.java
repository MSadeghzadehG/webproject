

package org.springframework.boot.context.properties.bind.test;

import java.util.ArrayList;
import java.util.List;

import org.junit.Before;
import org.junit.Test;

import org.springframework.boot.context.properties.bind.Bindable;
import org.springframework.boot.context.properties.bind.Binder;
import org.springframework.boot.context.properties.source.ConfigurationPropertyName;
import org.springframework.boot.context.properties.source.ConfigurationPropertySource;
import org.springframework.boot.context.properties.source.MockConfigurationPropertySource;

import static org.assertj.core.api.Assertions.assertThat;


public class PackagePrivateBeanBindingTests {

	private List<ConfigurationPropertySource> sources = new ArrayList<>();

	private Binder binder;

	private ConfigurationPropertyName name;

	@Before
	public void setup() {
		this.binder = new Binder(this.sources);
		this.name = ConfigurationPropertyName.of("foo");
	}

	@Test
	public void bindToPackagePrivateClassShouldBindToInstance() {
		MockConfigurationPropertySource source = new MockConfigurationPropertySource();
		source.put("foo.bar", "999");
		this.sources.add(source);
		ExamplePackagePrivateBean bean = this.binder
				.bind(this.name, Bindable.of(ExamplePackagePrivateBean.class)).get();
		assertThat(bean.getBar()).isEqualTo(999);
	}

	static class ExamplePackagePrivateBean {

		private int bar;

		public int getBar() {
			return this.bar;
		}

		public void setBar(int bar) {
			this.bar = bar;
		}

	}

}
