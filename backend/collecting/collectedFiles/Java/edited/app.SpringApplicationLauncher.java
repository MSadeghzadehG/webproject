

package org.springframework.boot.cli.app;

import java.lang.reflect.Constructor;
import java.lang.reflect.Method;
import java.util.HashMap;
import java.util.Map;


public class SpringApplicationLauncher {

	private static final String DEFAULT_SPRING_APPLICATION_CLASS = "org.springframework.boot.SpringApplication";

	private final ClassLoader classLoader;

	
	public SpringApplicationLauncher(ClassLoader classLoader) {
		this.classLoader = classLoader;
	}

	
	public Object launch(Class<?>[] sources, String[] args) throws Exception {
		Map<String, Object> defaultProperties = new HashMap<>();
		defaultProperties.put("spring.groovy.template.check-template-location", "false");
		Class<?> applicationClass = this.classLoader
				.loadClass(getSpringApplicationClassName());
		Constructor<?> constructor = applicationClass.getConstructor(Class[].class);
		Object application = constructor.newInstance((Object) sources);
		applicationClass.getMethod("setDefaultProperties", Map.class).invoke(application,
				defaultProperties);
		Method method = applicationClass.getMethod("run", String[].class);
		return method.invoke(application, (Object) args);
	}

	private String getSpringApplicationClassName() {
		String className = System.getProperty("spring.application.class.name");
		if (className == null) {
			className = getEnvironmentVariable("SPRING_APPLICATION_CLASS_NAME");
		}
		if (className == null) {
			className = DEFAULT_SPRING_APPLICATION_CLASS;
		}
		return className;
	}

	protected String getEnvironmentVariable(String name) {
		return System.getenv(name);
	}

}
