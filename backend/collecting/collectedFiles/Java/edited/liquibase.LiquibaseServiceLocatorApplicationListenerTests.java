

package org.springframework.boot.liquibase;

import java.lang.reflect.Field;
import java.net.URL;
import java.net.URLClassLoader;
import java.util.Arrays;
import java.util.List;

import liquibase.servicelocator.CustomResolverServiceLocator;
import liquibase.servicelocator.DefaultPackageScanClassResolver;
import liquibase.servicelocator.ServiceLocator;
import org.junit.After;
import org.junit.Test;

import org.springframework.boot.SpringApplication;
import org.springframework.boot.WebApplicationType;
import org.springframework.context.ConfigurableApplicationContext;
import org.springframework.context.annotation.Configuration;
import org.springframework.core.io.DefaultResourceLoader;
import org.springframework.util.ReflectionUtils;

import static org.assertj.core.api.Assertions.assertThat;


public class LiquibaseServiceLocatorApplicationListenerTests {

	private ConfigurableApplicationContext context;

	@After
	public void cleanUp() {
		if (this.context != null) {
			this.context.close();
		}
	}

	@Test
	public void replacesServiceLocator() throws IllegalAccessException {
		SpringApplication application = new SpringApplication(Conf.class);
		application.setWebApplicationType(WebApplicationType.NONE);
		this.context = application.run();
		Object resolver = getClassResolver();
		assertThat(resolver).isInstanceOf(SpringPackageScanClassResolver.class);
	}

	@Test
	public void replaceServiceLocatorBacksOffIfNotPresent()
			throws IllegalAccessException {
		SpringApplication application = new SpringApplication(Conf.class);
		application.setWebApplicationType(WebApplicationType.NONE);
		DefaultResourceLoader resourceLoader = new DefaultResourceLoader();
		resourceLoader.setClassLoader(
				new ClassHidingClassLoader(CustomResolverServiceLocator.class));
		application.setResourceLoader(resourceLoader);
		this.context = application.run();
		Object resolver = getClassResolver();
		assertThat(resolver).isInstanceOf(DefaultPackageScanClassResolver.class);
	}

	private Object getClassResolver() throws IllegalAccessException {
		ServiceLocator instance = ServiceLocator.getInstance();
		Field field = ReflectionUtils.findField(ServiceLocator.class, "classResolver");
		field.setAccessible(true);
		return field.get(instance);
	}

	@Configuration
	public static class Conf {

	}

	private final class ClassHidingClassLoader extends URLClassLoader {

		private final List<Class<?>> hiddenClasses;

		private ClassHidingClassLoader(Class<?>... hiddenClasses) {
			super(new URL[0], LiquibaseServiceLocatorApplicationListenerTests.class
					.getClassLoader());
			this.hiddenClasses = Arrays.asList(hiddenClasses);
		}

		@Override
		public Class<?> loadClass(String name) throws ClassNotFoundException {
			if (isHidden(name)) {
				throw new ClassNotFoundException();
			}
			return super.loadClass(name);
		}

		private boolean isHidden(String name) {
			for (Class<?> hiddenClass : this.hiddenClasses) {
				if (hiddenClass.getName().equals(name)) {
					return true;
				}
			}
			return false;
		}

	}

}
