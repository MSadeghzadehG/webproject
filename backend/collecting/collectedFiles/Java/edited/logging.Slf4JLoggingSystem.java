

package org.springframework.boot.logging;

import java.util.logging.ConsoleHandler;
import java.util.logging.Handler;
import java.util.logging.LogManager;
import java.util.logging.Logger;

import org.slf4j.bridge.SLF4JBridgeHandler;

import org.springframework.util.Assert;
import org.springframework.util.ClassUtils;


public abstract class Slf4JLoggingSystem extends AbstractLoggingSystem {

	private static final String BRIDGE_HANDLER = "org.slf4j.bridge.SLF4JBridgeHandler";

	public Slf4JLoggingSystem(ClassLoader classLoader) {
		super(classLoader);
	}

	@Override
	public void beforeInitialize() {
		super.beforeInitialize();
		configureJdkLoggingBridgeHandler();
	}

	@Override
	public void cleanUp() {
		removeJdkLoggingBridgeHandler();
	}

	@Override
	protected void loadConfiguration(LoggingInitializationContext initializationContext,
			String location, LogFile logFile) {
		Assert.notNull(location, "Location must not be null");
		if (initializationContext != null) {
			applySystemProperties(initializationContext.getEnvironment(), logFile);
		}
	}

	private void configureJdkLoggingBridgeHandler() {
		try {
			if (isBridgeHandlerAvailable()) {
				removeJdkLoggingBridgeHandler();
				SLF4JBridgeHandler.install();
			}
		}
		catch (Throwable ex) {
					}
	}

	protected final boolean isBridgeHandlerAvailable() {
		return ClassUtils.isPresent(BRIDGE_HANDLER, getClassLoader());
	}

	private void removeJdkLoggingBridgeHandler() {
		try {
			if (isBridgeHandlerAvailable()) {
				removeDefaultRootHandler();
				SLF4JBridgeHandler.uninstall();
			}
		}
		catch (Throwable ex) {
					}
	}

	private void removeDefaultRootHandler() {
		try {
			Logger rootLogger = LogManager.getLogManager().getLogger("");
			Handler[] handlers = rootLogger.getHandlers();
			if (handlers.length == 1 && handlers[0] instanceof ConsoleHandler) {
				rootLogger.removeHandler(handlers[0]);
			}
		}
		catch (Throwable ex) {
					}
	}

}
