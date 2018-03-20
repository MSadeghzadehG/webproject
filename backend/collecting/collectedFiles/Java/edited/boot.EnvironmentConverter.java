

package org.springframework.boot;

import java.util.Collections;
import java.util.HashSet;
import java.util.Set;

import org.springframework.core.env.ConfigurableEnvironment;
import org.springframework.core.env.Environment;
import org.springframework.core.env.MutablePropertySources;
import org.springframework.core.env.PropertySource;
import org.springframework.core.env.StandardEnvironment;
import org.springframework.util.ClassUtils;
import org.springframework.web.context.ConfigurableWebEnvironment;
import org.springframework.web.context.support.StandardServletEnvironment;


final class EnvironmentConverter {

	private static final String CONFIGURABLE_WEB_ENVIRONMENT_CLASS = "org.springframework.web.context.ConfigurableWebEnvironment";

	private static final Set<String> SERVLET_ENVIRONMENT_SOURCE_NAMES;

	static {
		final Set<String> names = new HashSet<>();
		names.add(StandardServletEnvironment.SERVLET_CONTEXT_PROPERTY_SOURCE_NAME);
		names.add(StandardServletEnvironment.SERVLET_CONFIG_PROPERTY_SOURCE_NAME);
		names.add(StandardServletEnvironment.JNDI_PROPERTY_SOURCE_NAME);
		SERVLET_ENVIRONMENT_SOURCE_NAMES = Collections.unmodifiableSet(names);
	}

	private final ClassLoader classLoader;

	
	EnvironmentConverter(ClassLoader classLoader) {
		this.classLoader = classLoader;
	}

	
	StandardEnvironment convertToStandardEnvironmentIfNecessary(
			ConfigurableEnvironment environment) {
		if (environment instanceof StandardEnvironment
				&& !isWebEnvironment(environment, this.classLoader)) {
			return (StandardEnvironment) environment;
		}
		return convertToStandardEnvironment(environment);
	}

	private boolean isWebEnvironment(ConfigurableEnvironment environment,
			ClassLoader classLoader) {
		try {
			Class<?> webEnvironmentClass = ClassUtils
					.forName(CONFIGURABLE_WEB_ENVIRONMENT_CLASS, classLoader);
			return (webEnvironmentClass.isInstance(environment));
		}
		catch (Throwable ex) {
			return false;
		}
	}

	private StandardEnvironment convertToStandardEnvironment(
			ConfigurableEnvironment environment) {
		StandardEnvironment result = new StandardEnvironment();
		result.setActiveProfiles(environment.getActiveProfiles());
		result.setConversionService(environment.getConversionService());
		copyNonServletPropertySources(environment, result);
		return result;
	}

	private void copyNonServletPropertySources(ConfigurableEnvironment source,
			StandardEnvironment target) {
		removeAllPropertySources(target.getPropertySources());
		for (PropertySource<?> propertySource : source.getPropertySources()) {
			if (!SERVLET_ENVIRONMENT_SOURCE_NAMES.contains(propertySource.getName())) {
				target.getPropertySources().addLast(propertySource);
			}
		}
	}

	private void removeAllPropertySources(MutablePropertySources propertySources) {
		Set<String> names = new HashSet<>();
		for (PropertySource<?> propertySource : propertySources) {
			names.add(propertySource.getName());
		}
		for (String name : names) {
			propertySources.remove(name);
		}
	}

}
