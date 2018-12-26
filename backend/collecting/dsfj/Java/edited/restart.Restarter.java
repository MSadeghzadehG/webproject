

package org.springframework.boot.devtools.restart;

import java.beans.Introspector;
import java.lang.Thread.UncaughtExceptionHandler;
import java.lang.reflect.Field;
import java.net.URL;
import java.util.Arrays;
import java.util.Collection;
import java.util.HashMap;
import java.util.LinkedHashSet;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.concurrent.BlockingDeque;
import java.util.concurrent.Callable;
import java.util.concurrent.CopyOnWriteArrayList;
import java.util.concurrent.LinkedBlockingDeque;
import java.util.concurrent.ThreadFactory;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import org.springframework.beans.CachedIntrospectionResults;
import org.springframework.beans.factory.ObjectFactory;
import org.springframework.boot.SpringApplication;
import org.springframework.boot.devtools.restart.FailureHandler.Outcome;
import org.springframework.boot.devtools.restart.classloader.ClassLoaderFiles;
import org.springframework.boot.devtools.restart.classloader.RestartClassLoader;
import org.springframework.boot.logging.DeferredLog;
import org.springframework.boot.system.JavaVersion;
import org.springframework.cglib.core.ClassNameReader;
import org.springframework.context.ConfigurableApplicationContext;
import org.springframework.context.support.GenericApplicationContext;
import org.springframework.core.ResolvableType;
import org.springframework.core.annotation.AnnotationUtils;
import org.springframework.core.io.ResourceLoader;
import org.springframework.util.Assert;
import org.springframework.util.ReflectionUtils;


public class Restarter {

	private static final Object INSTANCE_MONITOR = new Object();

	private static final String[] NO_ARGS = {};

	private static Restarter instance;

	private final Set<URL> urls = new LinkedHashSet<>();

	private final ClassLoaderFiles classLoaderFiles = new ClassLoaderFiles();

	private final Map<String, Object> attributes = new HashMap<>();

	private final BlockingDeque<LeakSafeThread> leakSafeThreads = new LinkedBlockingDeque<>();

	private final Lock stopLock = new ReentrantLock();

	private final Object monitor = new Object();

	private Log logger = new DeferredLog();

	private final boolean forceReferenceCleanup;

	private boolean enabled = true;

	private URL[] initialUrls;

	private final String mainClassName;

	private final ClassLoader applicationClassLoader;

	private final String[] args;

	private final UncaughtExceptionHandler exceptionHandler;

	private boolean finished = false;

	private final List<ConfigurableApplicationContext> rootContexts = new CopyOnWriteArrayList<>();

	
	protected Restarter(Thread thread, String[] args, boolean forceReferenceCleanup,
			RestartInitializer initializer) {
		Assert.notNull(thread, "Thread must not be null");
		Assert.notNull(args, "Args must not be null");
		Assert.notNull(initializer, "Initializer must not be null");
		this.logger.debug("Creating new Restarter for thread " + thread);
		SilentExitExceptionHandler.setup(thread);
		this.forceReferenceCleanup = forceReferenceCleanup;
		this.initialUrls = initializer.getInitialUrls(thread);
		this.mainClassName = getMainClassName(thread);
		this.applicationClassLoader = thread.getContextClassLoader();
		this.args = args;
		this.exceptionHandler = thread.getUncaughtExceptionHandler();
		this.leakSafeThreads.add(new LeakSafeThread());
	}

	private String getMainClassName(Thread thread) {
		try {
			return new MainMethod(thread).getDeclaringClassName();
		}
		catch (Exception ex) {
			return null;
		}
	}

	protected void initialize(boolean restartOnInitialize) {
		preInitializeLeakyClasses();
		if (this.initialUrls != null) {
			this.urls.addAll(Arrays.asList(this.initialUrls));
			if (restartOnInitialize) {
				this.logger.debug("Immediately restarting application");
				immediateRestart();
			}
		}
	}

	private void immediateRestart() {
		try {
			getLeakSafeThread().callAndWait(() -> {
				start(FailureHandler.NONE);
				cleanupCaches();
				return null;
			});
		}
		catch (Exception ex) {
			this.logger.warn("Unable to initialize restarter", ex);
		}
		SilentExitExceptionHandler.exitCurrentThread();
	}

	
	private void preInitializeLeakyClasses() {
		try {
			Class<?> readerClass = ClassNameReader.class;
			Field field = readerClass.getDeclaredField("EARLY_EXIT");
			field.setAccessible(true);
			((Throwable) field.get(null)).fillInStackTrace();
		}
		catch (Exception ex) {
			this.logger.warn("Unable to pre-initialize classes", ex);
		}
	}

	
	private void setEnabled(boolean enabled) {
		this.enabled = enabled;
	}

	
	public void addUrls(Collection<URL> urls) {
		Assert.notNull(urls, "Urls must not be null");
		this.urls.addAll(urls);
	}

	
	public void addClassLoaderFiles(ClassLoaderFiles classLoaderFiles) {
		Assert.notNull(classLoaderFiles, "ClassLoaderFiles must not be null");
		this.classLoaderFiles.addAll(classLoaderFiles);
	}

	
	public ThreadFactory getThreadFactory() {
		return new LeakSafeThreadFactory();
	}

	
	public void restart() {
		restart(FailureHandler.NONE);
	}

	
	public void restart(FailureHandler failureHandler) {
		if (!this.enabled) {
			this.logger.debug("Application restart is disabled");
			return;
		}
		this.logger.debug("Restarting application");
		getLeakSafeThread().call(() -> {
			Restarter.this.stop();
			Restarter.this.start(failureHandler);
			return null;
		});
	}

	
	protected void start(FailureHandler failureHandler) throws Exception {
		do {
			Throwable error = doStart();
			if (error == null) {
				return;
			}
			if (failureHandler.handle(error) == Outcome.ABORT) {
				return;
			}
		}
		while (true);
	}

	private Throwable doStart() throws Exception {
		Assert.notNull(this.mainClassName, "Unable to find the main class to restart");
		URL[] urls = this.urls.toArray(new URL[0]);
		ClassLoaderFiles updatedFiles = new ClassLoaderFiles(this.classLoaderFiles);
		ClassLoader classLoader = new RestartClassLoader(this.applicationClassLoader,
				urls, updatedFiles, this.logger);
		if (this.logger.isDebugEnabled()) {
			this.logger.debug("Starting application " + this.mainClassName + " with URLs "
					+ Arrays.asList(urls));
		}
		return relaunch(classLoader);
	}

	
	protected Throwable relaunch(ClassLoader classLoader) throws Exception {
		RestartLauncher launcher = new RestartLauncher(classLoader, this.mainClassName,
				this.args, this.exceptionHandler);
		launcher.start();
		launcher.join();
		return launcher.getError();
	}

	
	protected void stop() throws Exception {
		this.logger.debug("Stopping application");
		this.stopLock.lock();
		try {
			for (ConfigurableApplicationContext context : this.rootContexts) {
				context.close();
				this.rootContexts.remove(context);
			}
			cleanupCaches();
			if (this.forceReferenceCleanup) {
				forceReferenceCleanup();
			}
		}
		finally {
			this.stopLock.unlock();
		}
		System.gc();
		System.runFinalization();
	}

	private void cleanupCaches() throws Exception {
		Introspector.flushCaches();
		cleanupKnownCaches();
	}

	private void cleanupKnownCaches() throws Exception {
						clear(ResolvableType.class, "cache");
		clear("org.springframework.core.SerializableTypeWrapper", "cache");
		clear(CachedIntrospectionResults.class, "acceptedClassLoaders");
		clear(CachedIntrospectionResults.class, "strongClassCache");
		clear(CachedIntrospectionResults.class, "softClassCache");
		clear(ReflectionUtils.class, "declaredFieldsCache");
		clear(ReflectionUtils.class, "declaredMethodsCache");
		clear(AnnotationUtils.class, "findAnnotationCache");
		clear(AnnotationUtils.class, "annotatedInterfaceCache");
		if (!JavaVersion.getJavaVersion().isEqualOrNewerThan(JavaVersion.NINE)) {
			clear("com.sun.naming.internal.ResourceManager", "propertiesCache");
		}
	}

	private void clear(String className, String fieldName) {
		try {
			clear(Class.forName(className), fieldName);
		}
		catch (Exception ex) {
			this.logger.debug("Unable to clear field " + className + " " + fieldName, ex);
		}
	}

	private void clear(Class<?> type, String fieldName) throws Exception {
		Field field = type.getDeclaredField(fieldName);
		field.setAccessible(true);
		Object instance = field.get(null);
		if (instance instanceof Set) {
			((Set<?>) instance).clear();
		}
		if (instance instanceof Map) {
			((Map<?, ?>) instance).keySet().removeIf(this::isFromRestartClassLoader);
		}
	}

	private boolean isFromRestartClassLoader(Object object) {
		if (object instanceof Class) {
			return ((Class<?>) object).getClassLoader() instanceof RestartClassLoader;
		}
		return false;
	}

	
	private void forceReferenceCleanup() {
		try {
			final List<long[]> memory = new LinkedList<>();
			while (true) {
				memory.add(new long[102400]);
			}
		}
		catch (OutOfMemoryError ex) {
					}
	}

	
	void finish() {
		synchronized (this.monitor) {
			if (!isFinished()) {
				this.logger = DeferredLog.replay(this.logger,
						LogFactory.getLog(getClass()));
				this.finished = true;
			}
		}
	}

	boolean isFinished() {
		synchronized (this.monitor) {
			return this.finished;
		}
	}

	void prepare(ConfigurableApplicationContext applicationContext) {
		if (applicationContext != null && applicationContext.getParent() != null) {
			return;
		}
		if (applicationContext instanceof GenericApplicationContext) {
			prepare((GenericApplicationContext) applicationContext);
		}
		this.rootContexts.add(applicationContext);
	}

	void remove(ConfigurableApplicationContext applicationContext) {
		if (applicationContext != null) {
			this.rootContexts.remove(applicationContext);
		}
	}

	private void prepare(GenericApplicationContext applicationContext) {
		ResourceLoader resourceLoader = new ClassLoaderFilesResourcePatternResolver(
				applicationContext, this.classLoaderFiles);
		applicationContext.setResourceLoader(resourceLoader);
	}

	private LeakSafeThread getLeakSafeThread() {
		try {
			return this.leakSafeThreads.takeFirst();
		}
		catch (InterruptedException ex) {
			Thread.currentThread().interrupt();
			throw new IllegalStateException(ex);
		}
	}

	public Object getOrAddAttribute(String name, final ObjectFactory<?> objectFactory) {
		synchronized (this.attributes) {
			if (!this.attributes.containsKey(name)) {
				this.attributes.put(name, objectFactory.getObject());
			}
			return this.attributes.get(name);
		}
	}

	public Object removeAttribute(String name) {
		synchronized (this.attributes) {
			return this.attributes.remove(name);
		}
	}

	
	public URL[] getInitialUrls() {
		return this.initialUrls;
	}

	
	public static void disable() {
		initialize(NO_ARGS, false, RestartInitializer.NONE);
		getInstance().setEnabled(false);
	}

	
	public static void initialize(String[] args) {
		initialize(args, false, new DefaultRestartInitializer());
	}

	
	public static void initialize(String[] args, RestartInitializer initializer) {
		initialize(args, false, initializer, true);
	}

	
	public static void initialize(String[] args, boolean forceReferenceCleanup) {
		initialize(args, forceReferenceCleanup, new DefaultRestartInitializer());
	}

	
	public static void initialize(String[] args, boolean forceReferenceCleanup,
			RestartInitializer initializer) {
		initialize(args, forceReferenceCleanup, initializer, true);
	}

	
	public static void initialize(String[] args, boolean forceReferenceCleanup,
			RestartInitializer initializer, boolean restartOnInitialize) {
		Restarter localInstance = null;
		synchronized (INSTANCE_MONITOR) {
			if (instance == null) {
				localInstance = new Restarter(Thread.currentThread(), args,
						forceReferenceCleanup, initializer);
				instance = localInstance;
			}
		}
		if (localInstance != null) {
			localInstance.initialize(restartOnInitialize);
		}
	}

	
	public static Restarter getInstance() {
		synchronized (INSTANCE_MONITOR) {
			Assert.state(instance != null, "Restarter has not been initialized");
			return instance;
		}
	}

	
	static void setInstance(Restarter instance) {
		synchronized (INSTANCE_MONITOR) {
			Restarter.instance = instance;
		}
	}

	
	public static void clearInstance() {
		synchronized (INSTANCE_MONITOR) {
			instance = null;
		}
	}

	
	private class LeakSafeThread extends Thread {

		private Callable<?> callable;

		private Object result;

		LeakSafeThread() {
			setDaemon(false);
		}

		public void call(Callable<?> callable) {
			this.callable = callable;
			start();
		}

		@SuppressWarnings("unchecked")
		public <V> V callAndWait(Callable<V> callable) {
			this.callable = callable;
			start();
			try {
				join();
				return (V) this.result;
			}
			catch (InterruptedException ex) {
				Thread.currentThread().interrupt();
				throw new IllegalStateException(ex);
			}
		}

		@Override
		public void run() {
												try {
				Restarter.this.leakSafeThreads.put(new LeakSafeThread());
				this.result = this.callable.call();
			}
			catch (Exception ex) {
				ex.printStackTrace();
				System.exit(1);
			}
		}

	}

	
	private class LeakSafeThreadFactory implements ThreadFactory {

		@Override
		public Thread newThread(Runnable runnable) {
			return getLeakSafeThread().callAndWait(() -> {
				Thread thread = new Thread(runnable);
				thread.setContextClassLoader(Restarter.this.applicationClassLoader);
				return thread;
			});
		}

	}

}
