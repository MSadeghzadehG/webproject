

package org.springframework.boot.configurationsample.specific;

import java.util.HashMap;
import java.util.Map;

import org.springframework.boot.configurationsample.ConfigurationProperties;
import org.springframework.boot.configurationsample.NestedConfigurationProperty;


@ConfigurationProperties("generic")
public class GenericConfig<T> {

	private final Foo foo = new Foo();

	public Foo getFoo() {
		return this.foo;
	}

	public static class Foo {

		private String name;

		@NestedConfigurationProperty
		private final Bar<String> bar = new Bar<>();

		private final Map<String, Bar<Integer>> stringToBar = new HashMap<>();

		private final Map<String, Integer> stringToInteger = new HashMap<>();

		public String getName() {
			return this.name;
		}

		public void setName(String name) {
			this.name = name;
		}

		public Bar<String> getBar() {
			return this.bar;
		}

		public Map<String, Bar<Integer>> getStringToBar() {
			return this.stringToBar;
		}

		public Map<String, Integer> getStringToInteger() {
			return this.stringToInteger;
		}

	}

	public static class Bar<U> {

		private String name;

		@NestedConfigurationProperty
		private final Biz<String> biz = new Biz<>();

		public String getName() {
			return this.name;
		}

		public void setName(String name) {
			this.name = name;
		}

		public Biz<String> getBiz() {
			return this.biz;
		}

		public static class Biz<V> {

			private String name;

			public String getName() {
				return this.name;
			}

			public void setName(String name) {
				this.name = name;
			}

		}

	}

}
