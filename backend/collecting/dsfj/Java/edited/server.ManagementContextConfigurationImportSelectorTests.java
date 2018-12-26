

package org.springframework.boot.actuate.autoconfigure.web.server;

import java.util.List;
import java.util.stream.Collectors;
import java.util.stream.Stream;

import org.junit.Test;

import org.springframework.boot.actuate.autoconfigure.web.ManagementContextConfiguration;
import org.springframework.boot.actuate.autoconfigure.web.ManagementContextType;
import org.springframework.core.annotation.Order;
import org.springframework.core.type.StandardAnnotationMetadata;

import static org.assertj.core.api.Assertions.assertThat;


public class ManagementContextConfigurationImportSelectorTests {

	@Test
	public void selectImportsShouldOrderResult() {
		String[] imports = new TestManagementContextConfigurationsImportSelector(C.class,
				A.class, D.class, B.class).selectImports(
						new StandardAnnotationMetadata(EnableChildContext.class));
		assertThat(imports).containsExactly(A.class.getName(), B.class.getName(),
				C.class.getName(), D.class.getName());
	}

	@Test
	public void selectImportsFiltersChildOnlyConfigurationWhenUsingSameContext() {
		String[] imports = new TestManagementContextConfigurationsImportSelector(
				ChildOnly.class, SameOnly.class, A.class).selectImports(
						new StandardAnnotationMetadata(EnableSameContext.class));
		assertThat(imports).containsExactlyInAnyOrder(SameOnly.class.getName(),
				A.class.getName());
	}

	@Test
	public void selectImportsFiltersSameOnlyConfigurationWhenUsingChildContext() {
		String[] imports = new TestManagementContextConfigurationsImportSelector(
				ChildOnly.class, SameOnly.class, A.class).selectImports(
						new StandardAnnotationMetadata(EnableChildContext.class));
		assertThat(imports).containsExactlyInAnyOrder(ChildOnly.class.getName(),
				A.class.getName());
	}

	private static final class TestManagementContextConfigurationsImportSelector
			extends ManagementContextConfigurationImportSelector {

		private final List<String> factoryNames;

		private TestManagementContextConfigurationsImportSelector(Class<?>... classes) {
			this.factoryNames = Stream.of(classes).map(Class::getName)
					.collect(Collectors.toList());
		}

		@Override
		protected List<String> loadFactoryNames() {
			return this.factoryNames;
		}

	}

	@Order(1)
	private static class A {

	}

	@Order(2)
	private static class B {

	}

	@Order(3)
	private static class C {

	}

	static class D {

	}

	@ManagementContextConfiguration(ManagementContextType.CHILD)
	static class ChildOnly {

	}

	@ManagementContextConfiguration(ManagementContextType.SAME)
	static class SameOnly {

	}

	@EnableManagementContext(ManagementContextType.CHILD)
	static class EnableChildContext {

	}

	@EnableManagementContext(ManagementContextType.SAME)
	static class EnableSameContext {

	}

}
