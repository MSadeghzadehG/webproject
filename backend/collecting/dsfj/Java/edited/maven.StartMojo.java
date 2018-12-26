

package org.springframework.boot.maven;

import java.io.File;
import java.io.IOException;
import java.lang.management.ManagementFactory;
import java.net.ConnectException;
import java.net.URLClassLoader;
import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.Callable;

import javax.management.MBeanServerConnection;
import javax.management.ReflectionException;
import javax.management.remote.JMXConnector;

import org.apache.maven.plugin.MojoExecutionException;
import org.apache.maven.plugin.MojoFailureException;
import org.apache.maven.plugins.annotations.LifecyclePhase;
import org.apache.maven.plugins.annotations.Mojo;
import org.apache.maven.plugins.annotations.Parameter;
import org.apache.maven.plugins.annotations.ResolutionScope;

import org.springframework.boot.loader.tools.JavaExecutable;
import org.springframework.boot.loader.tools.RunProcess;


@Mojo(name = "start", requiresProject = true, defaultPhase = LifecyclePhase.PRE_INTEGRATION_TEST, requiresDependencyResolution = ResolutionScope.TEST)
public class StartMojo extends AbstractRunMojo {

	private static final String ENABLE_MBEAN_PROPERTY = "--spring.application.admin.enabled=true";

	private static final String JMX_NAME_PROPERTY_PREFIX = "--spring.application.admin.jmx-name=";

	
	@Parameter
	private String jmxName = SpringApplicationAdminClient.DEFAULT_OBJECT_NAME;

	
	@Parameter
	private int jmxPort = 9001;

	
	@Parameter
	private long wait = 500;

	
	@Parameter
	private int maxAttempts = 60;

	private final Object lock = new Object();

	@Override
	protected void runWithForkedJvm(File workingDirectory, List<String> args)
			throws MojoExecutionException, MojoFailureException {
		RunProcess runProcess = runProcess(workingDirectory, args);
		try {
			waitForSpringApplication();
		}
		catch (MojoExecutionException | MojoFailureException ex) {
			runProcess.kill();
			throw ex;
		}
	}

	private RunProcess runProcess(File workingDirectory, List<String> args)
			throws MojoExecutionException {
		try {
			RunProcess runProcess = new RunProcess(workingDirectory,
					new JavaExecutable().toString());
			runProcess.run(false, args.toArray(new String[0]));
			return runProcess;
		}
		catch (Exception ex) {
			throw new MojoExecutionException("Could not exec java", ex);
		}
	}

	@Override
	protected RunArguments resolveApplicationArguments() {
		RunArguments applicationArguments = super.resolveApplicationArguments();
		applicationArguments.getArgs().addLast(ENABLE_MBEAN_PROPERTY);
		if (isFork()) {
			applicationArguments.getArgs()
					.addLast(JMX_NAME_PROPERTY_PREFIX + this.jmxName);
		}
		return applicationArguments;
	}

	@Override
	protected RunArguments resolveJvmArguments() {
		RunArguments jvmArguments = super.resolveJvmArguments();
		if (isFork()) {
			List<String> remoteJmxArguments = new ArrayList<>();
			remoteJmxArguments.add("-Dcom.sun.management.jmxremote");
			remoteJmxArguments.add("-Dcom.sun.management.jmxremote.port=" + this.jmxPort);
			remoteJmxArguments.add("-Dcom.sun.management.jmxremote.authenticate=false");
			remoteJmxArguments.add("-Dcom.sun.management.jmxremote.ssl=false");
			jvmArguments.getArgs().addAll(remoteJmxArguments);
		}
		return jvmArguments;
	}

	@Override
	protected void runWithMavenJvm(String startClassName, String... arguments)
			throws MojoExecutionException {
		IsolatedThreadGroup threadGroup = new IsolatedThreadGroup(startClassName);
		Thread launchThread = new Thread(threadGroup,
				new LaunchRunner(startClassName, arguments), startClassName + ".main()");
		launchThread.setContextClassLoader(new URLClassLoader(getClassPathUrls()));
		launchThread.start();
		waitForSpringApplication(this.wait, this.maxAttempts);
	}

	private void waitForSpringApplication(long wait, int maxAttempts)
			throws MojoExecutionException {
		SpringApplicationAdminClient client = new SpringApplicationAdminClient(
				ManagementFactory.getPlatformMBeanServer(), this.jmxName);
		getLog().debug("Waiting for spring application to start...");
		for (int i = 0; i < maxAttempts; i++) {
			if (client.isReady()) {
				return;
			}
			String message = "Spring application is not ready yet, waiting " + wait
					+ "ms (attempt " + (i + 1) + ")";
			getLog().debug(message);
			synchronized (this.lock) {
				try {
					this.lock.wait(wait);
				}
				catch (InterruptedException ex) {
					Thread.currentThread().interrupt();
					throw new IllegalStateException(
							"Interrupted while waiting for Spring Boot app to start.");
				}
			}
		}
		throw new MojoExecutionException(
				"Spring application did not start before the configured timeout ("
						+ (wait * maxAttempts) + "ms");
	}

	private void waitForSpringApplication()
			throws MojoFailureException, MojoExecutionException {
		try {
			if (isFork()) {
				waitForForkedSpringApplication();
			}
			else {
				doWaitForSpringApplication(ManagementFactory.getPlatformMBeanServer());
			}
		}
		catch (IOException ex) {
			throw new MojoFailureException("Could not contact Spring Boot application",
					ex);
		}
		catch (Exception ex) {
			throw new MojoExecutionException(
					"Could not figure out if the application has started", ex);
		}
	}

	private void waitForForkedSpringApplication()
			throws IOException, MojoFailureException, MojoExecutionException {
		try {
			getLog().debug("Connecting to local MBeanServer at port " + this.jmxPort);
			try (JMXConnector connector = execute(this.wait, this.maxAttempts,
					new CreateJmxConnector(this.jmxPort))) {
				if (connector == null) {
					throw new MojoExecutionException(
							"JMX MBean server was not reachable before the configured "
									+ "timeout (" + (this.wait * this.maxAttempts)
									+ "ms");
				}
				getLog().debug("Connected to local MBeanServer at port " + this.jmxPort);
				MBeanServerConnection connection = connector.getMBeanServerConnection();
				doWaitForSpringApplication(connection);
			}
		}
		catch (IOException ex) {
			throw ex;
		}
		catch (Exception ex) {
			throw new MojoExecutionException(
					"Failed to connect to MBean server at port " + this.jmxPort, ex);
		}
	}

	private void doWaitForSpringApplication(MBeanServerConnection connection)
			throws IOException, MojoExecutionException, MojoFailureException {
		final SpringApplicationAdminClient client = new SpringApplicationAdminClient(
				connection, this.jmxName);
		try {
			execute(this.wait, this.maxAttempts, () -> (client.isReady() ? true : null));
		}
		catch (ReflectionException ex) {
			throw new MojoExecutionException("Unable to retrieve 'ready' attribute",
					ex.getCause());
		}
		catch (Exception ex) {
			throw new MojoFailureException("Could not invoke shutdown operation", ex);
		}
	}

	
	public <T> T execute(long wait, int maxAttempts, Callable<T> callback)
			throws Exception {
		getLog().debug("Waiting for spring application to start...");
		for (int i = 0; i < maxAttempts; i++) {
			T result = callback.call();
			if (result != null) {
				return result;
			}
			String message = "Spring application is not ready yet, waiting " + wait
					+ "ms (attempt " + (i + 1) + ")";
			getLog().debug(message);
			synchronized (this.lock) {
				try {
					this.lock.wait(wait);
				}
				catch (InterruptedException ex) {
					Thread.currentThread().interrupt();
					throw new IllegalStateException(
							"Interrupted while waiting for Spring Boot app to start.");
				}
			}
		}
		throw new MojoExecutionException(
				"Spring application did not start before the configured " + "timeout ("
						+ (wait * maxAttempts) + "ms");
	}

	private class CreateJmxConnector implements Callable<JMXConnector> {

		private final int port;

		CreateJmxConnector(int port) {
			this.port = port;
		}

		@Override
		public JMXConnector call() throws Exception {
			try {
				return SpringApplicationAdminClient.connect(this.port);
			}
			catch (IOException ex) {
				if (hasCauseWithType(ex, ConnectException.class)) {
					String message = "MBean server at port " + this.port
							+ " is not up yet...";
					getLog().debug(message);
					return null;
				}
				throw ex;
			}
		}

		private boolean hasCauseWithType(Throwable t, Class<? extends Exception> type) {
			return type.isAssignableFrom(t.getClass())
					|| t.getCause() != null && hasCauseWithType(t.getCause(), type);
		}

	}

}
