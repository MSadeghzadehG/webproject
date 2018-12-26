

package org.springframework.boot.cli.compiler;

import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.ExpectedException;

import static org.assertj.core.api.Assertions.assertThat;


public class ExtendedGroovyClassLoaderTests {

	@Rule
	public ExpectedException thrown = ExpectedException.none();

	private ClassLoader contextClassLoader;

	private ExtendedGroovyClassLoader defaultScopeGroovyClassLoader;

	@Before
	public void setup() {
		this.contextClassLoader = Thread.currentThread().getContextClassLoader();
		this.defaultScopeGroovyClassLoader = new ExtendedGroovyClassLoader(
				GroovyCompilerScope.DEFAULT);
	}

	@Test
	public void loadsGroovyFromSameClassLoader() throws Exception {
		Class<?> c1 = this.contextClassLoader.loadClass("groovy.lang.Script");
		Class<?> c2 = this.defaultScopeGroovyClassLoader.loadClass("groovy.lang.Script");
		assertThat(c1.getClassLoader()).isSameAs(c2.getClassLoader());
	}

	@Test
	public void filtersNonGroovy() throws Exception {
		this.contextClassLoader.loadClass("org.springframework.util.StringUtils");
		this.thrown.expect(ClassNotFoundException.class);
		this.defaultScopeGroovyClassLoader
				.loadClass("org.springframework.util.StringUtils");
	}

	@Test
	public void loadsJavaTypes() throws Exception {
		this.defaultScopeGroovyClassLoader.loadClass("java.lang.Boolean");
	}

	@Test
	public void loadsSqlTypes() throws Exception {
		this.contextClassLoader.loadClass("java.sql.SQLException");
		this.defaultScopeGroovyClassLoader.loadClass("java.sql.SQLException");
	}

}
