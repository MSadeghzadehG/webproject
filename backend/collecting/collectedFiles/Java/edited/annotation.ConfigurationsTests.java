

package org.springframework.boot.context.annotation;

import java.io.InputStream;
import java.io.OutputStream;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.Comparator;
import java.util.Set;

import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.ExpectedException;

import org.springframework.core.Ordered;
import org.springframework.core.annotation.Order;
import org.springframework.util.ClassUtils;

import static org.assertj.core.api.Assertions.assertThat;


public class ConfigurationsTests {

	@Rule
	public ExpectedException thrown = ExpectedException.none();

	@Test
	public void createWhenClassesIsNullShouldThrowException() {
		this.thrown.expect(IllegalArgumentException.class);
		this.thrown.expectMessage("Classes must not be null");
		new TestConfigurations(null);
	}

	@Test
	public void createShouldSortClasses() {
		TestSortedConfigurations configurations = new TestSortedConfigurations(
				Arrays.asList(OutputStream.class, InputStream.class));
		assertThat(configurations.getClasses()).containsExactly(InputStream.class,
				OutputStream.class);
	}

	@Test
	public void getClassesShouldMergeByClassAndSort() {
		Configurations c1 = new TestSortedConfigurations(
				Arrays.asList(OutputStream.class, InputStream.class));
		Configurations c2 = new TestConfigurations(Arrays.asList(Short.class));
		Configurations c3 = new TestSortedConfigurations(
				Arrays.asList(String.class, Integer.class));
		Configurations c4 = new TestConfigurations(Arrays.asList(Long.class, Byte.class));
		Class<?>[] classes = Configurations.getClasses(c1, c2, c3, c4);
		assertThat(classes).containsExactly(Short.class, Long.class, Byte.class,
				InputStream.class, Integer.class, OutputStream.class, String.class);
	}

	@Order(Ordered.HIGHEST_PRECEDENCE)
	static class TestConfigurations extends Configurations {

		protected TestConfigurations(Collection<Class<?>> classes) {
			super(classes);
		}

		@Override
		protected Configurations merge(Set<Class<?>> mergedClasses) {
			return new TestConfigurations(mergedClasses);
		}

	}

	@Order(Ordered.LOWEST_PRECEDENCE)
	static class TestSortedConfigurations extends Configurations {

		protected TestSortedConfigurations(Collection<Class<?>> classes) {
			super(classes);
		}

		@Override
		protected Collection<Class<?>> sort(Collection<Class<?>> classes) {
			ArrayList<Class<?>> sorted = new ArrayList<>(classes);
			sorted.sort(Comparator.comparing(ClassUtils::getShortName));
			return sorted;
		}

		@Override
		protected Configurations merge(Set<Class<?>> mergedClasses) {
			return new TestSortedConfigurations(mergedClasses);
		}

	}

}
