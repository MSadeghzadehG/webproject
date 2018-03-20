

package org.springframework.boot.logging;

import java.util.Comparator;
import java.util.EnumMap;
import java.util.HashMap;
import java.util.LinkedHashSet;
import java.util.Map;
import java.util.Set;

import org.springframework.core.env.Environment;
import org.springframework.core.io.ClassPathResource;
import org.springframework.util.ClassUtils;
import org.springframework.util.StringUtils;
import org.springframework.util.SystemPropertyUtils;


public abstract class AbstractLoggingSystem extends LoggingSystem {

	protected static final Comparator<LoggerConfiguration> CONFIGURATION_COMPARATOR = new LoggerConfigurationComparator(
			ROOT_LOGGER_NAME);

	private final ClassLoader classLoader;

	public AbstractLoggingSystem(ClassLoader classLoader) {
		this.classLoader = classLoader;
	}

	@Override
	public void beforeInitialize() {
	}

	@Override
	public void initialize(LoggingInitializationContext initializationContext,
			String configLocation, LogFile logFile) {
		if (StringUtils.hasLength(configLocation)) {
			initializeWithSpecificConfig(initializationContext, configLocation, logFile);
			return;
		}
		initializeWithConventions(initializationContext, logFile);
	}

	private void initializeWithSpecificConfig(
			LoggingInitializationContext initializationContext, String configLocation,
			LogFile logFile) {
		configLocation = SystemPropertyUtils.resolvePlaceholders(configLocation);
		loadConfiguration(initializationContext, configLocation, logFile);
	}

	private void initializeWithConventions(
			LoggingInitializationContext initializationContext, LogFile logFile) {
		String config = getSelfInitializationConfig();
		if (config != null && logFile == null) {
						reinitialize(initializationContext);
			return;
		}
		if (config == null) {
			config = getSpringInitializationConfig();
		}
		if (config != null) {
			loadConfiguration(initializationContext, config, logFile);
			return;
		}
		loadDefaults(initializationContext, logFile);
	}

	
	protected String getSelfInitializationConfig() {
		return findConfig(getStandardConfigLocations());
	}

	
	protected String getSpringInitializationConfig() {
		return findConfig(getSpringConfigLocations());
	}

	private String findConfig(String[] locations) {
		for (String location : locations) {
			ClassPathResource resource = new ClassPathResource(location,
					this.classLoader);
			if (resource.exists()) {
				return "classpath:" + location;
			}
		}
		return null;
	}

	
	protected abstract String[] getStandardConfigLocations();

	
	protected String[] getSpringConfigLocations() {
		String[] locations = getStandardConfigLocations();
		for (int i = 0; i < locations.length; i++) {
			String extension = StringUtils.getFilenameExtension(locations[i]);
			locations[i] = locations[i].substring(0,
					locations[i].length() - extension.length() - 1) + "-spring."
					+ extension;
		}
		return locations;
	}

	
	protected abstract void loadDefaults(
			LoggingInitializationContext initializationContext, LogFile logFile);

	
	protected abstract void loadConfiguration(
			LoggingInitializationContext initializationContext, String location,
			LogFile logFile);

	
	protected void reinitialize(LoggingInitializationContext initializationContext) {
	}

	protected final ClassLoader getClassLoader() {
		return this.classLoader;
	}

	protected final String getPackagedConfigFile(String fileName) {
		String defaultPath = ClassUtils.getPackageName(getClass());
		defaultPath = defaultPath.replace('.', '/');
		defaultPath = defaultPath + "/" + fileName;
		defaultPath = "classpath:" + defaultPath;
		return defaultPath;
	}

	protected final void applySystemProperties(Environment environment, LogFile logFile) {
		new LoggingSystemProperties(environment).apply(logFile);
	}

	
	protected static class LogLevels<T> {

		private final Map<LogLevel, T> systemToNative;

		private final Map<T, LogLevel> nativeToSystem;

		public LogLevels() {
			this.systemToNative = new EnumMap<>(LogLevel.class);
			this.nativeToSystem = new HashMap<>();
		}

		public void map(LogLevel system, T nativeLevel) {
			if (!this.systemToNative.containsKey(system)) {
				this.systemToNative.put(system, nativeLevel);
			}
			if (!this.nativeToSystem.containsKey(nativeLevel)) {
				this.nativeToSystem.put(nativeLevel, system);
			}
		}

		public LogLevel convertNativeToSystem(T level) {
			return this.nativeToSystem.get(level);
		}

		public T convertSystemToNative(LogLevel level) {
			return this.systemToNative.get(level);
		}

		public Set<LogLevel> getSupported() {
			return new LinkedHashSet<>(this.nativeToSystem.values());
		}

	}

}
