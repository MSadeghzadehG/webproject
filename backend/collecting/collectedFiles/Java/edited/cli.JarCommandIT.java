

package org.springframework.boot.cli;

import java.io.File;

import org.junit.Test;

import org.springframework.boot.cli.command.archive.JarCommand;
import org.springframework.boot.cli.infrastructure.CommandLineInvoker;
import org.springframework.boot.cli.infrastructure.CommandLineInvoker.Invocation;
import org.springframework.boot.loader.tools.JavaExecutable;

import static org.hamcrest.Matchers.containsString;
import static org.hamcrest.Matchers.equalTo;
import static org.hamcrest.Matchers.not;
import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertThat;
import static org.junit.Assert.assertTrue;


public class JarCommandIT {

	private static final boolean JAVA_9_OR_LATER = isClassPresent(
			"java.security.cert.URICertStoreParameters");

	private final CommandLineInvoker cli = new CommandLineInvoker(
			new File("src/it/resources/jar-command"));

	@Test
	public void noArguments() throws Exception {
		Invocation invocation = this.cli.invoke("jar");
		invocation.await();
		assertThat(invocation.getStandardOutput(), equalTo(""));
		assertThat(invocation.getErrorOutput(), containsString("The name of the "
				+ "resulting jar and at least one source file must be specified"));
	}

	@Test
	public void noSources() throws Exception {
		Invocation invocation = this.cli.invoke("jar", "test-app.jar");
		invocation.await();
		assertThat(invocation.getStandardOutput(), equalTo(""));
		assertThat(invocation.getErrorOutput(), containsString("The name of the "
				+ "resulting jar and at least one source file must be specified"));
	}

	@Test
	public void jarCreationWithGrabResolver() throws Exception {
		File jar = new File("target/test-app.jar");
		Invocation invocation = this.cli.invoke("run", jar.getAbsolutePath(),
				"bad.groovy");
		invocation.await();
		if (!JAVA_9_OR_LATER) {
			assertThat(invocation.getErrorOutput(), equalTo(""));
		}
		invocation = this.cli.invoke("jar", jar.getAbsolutePath(), "bad.groovy");
		invocation.await();
		if (!JAVA_9_OR_LATER) {
			assertEquals(invocation.getErrorOutput(), 0,
					invocation.getErrorOutput().length());
		}
		assertTrue(jar.exists());

		Process process = new JavaExecutable()
				.processBuilder("-jar", jar.getAbsolutePath()).start();
		invocation = new Invocation(process);
		invocation.await();

		if (!JAVA_9_OR_LATER) {
			assertThat(invocation.getErrorOutput(), equalTo(""));
		}
	}

	@Test
	public void jarCreation() throws Exception {
		File jar = new File("target/test-app.jar");
		Invocation invocation = this.cli.invoke("jar", jar.getAbsolutePath(),
				"jar.groovy");
		invocation.await();
		if (!JAVA_9_OR_LATER) {
			assertEquals(invocation.getErrorOutput(), 0,
					invocation.getErrorOutput().length());
		}
		assertTrue(jar.exists());

		Process process = new JavaExecutable()
				.processBuilder("-jar", jar.getAbsolutePath()).start();
		invocation = new Invocation(process);
		invocation.await();

		if (!JAVA_9_OR_LATER) {
			assertThat(invocation.getErrorOutput(), equalTo(""));
		}
		assertThat(invocation.getStandardOutput(), containsString("Hello World!"));
		assertThat(invocation.getStandardOutput(),
				containsString("/BOOT-INF/classes!/public/public.txt"));
		assertThat(invocation.getStandardOutput(),
				containsString("/BOOT-INF/classes!/resources/resource.txt"));
		assertThat(invocation.getStandardOutput(),
				containsString("/BOOT-INF/classes!/static/static.txt"));
		assertThat(invocation.getStandardOutput(),
				containsString("/BOOT-INF/classes!/templates/template.txt"));
		assertThat(invocation.getStandardOutput(),
				containsString("/BOOT-INF/classes!/root.properties"));
		assertThat(invocation.getStandardOutput(), containsString("Goodbye Mama"));
	}

	@Test
	public void jarCreationWithIncludes() throws Exception {
		File jar = new File("target/test-app.jar");
		Invocation invocation = this.cli.invoke("jar", jar.getAbsolutePath(), "--include",
				"-public