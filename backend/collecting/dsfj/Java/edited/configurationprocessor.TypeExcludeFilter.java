

package org.springframework.boot.configurationprocessor;

import java.util.HashSet;
import java.util.Set;

import javax.lang.model.type.TypeMirror;


class TypeExcludeFilter {

	private final Set<String> excludes = new HashSet<>();

	TypeExcludeFilter() {
		add("com.zaxxer.hikari.IConnectionCustomizer");
		add("groovy.text.markup.MarkupTemplateEngine");
		add("java.io.Writer");
		add("java.io.PrintWriter");
		add("java.lang.ClassLoader");
		add("java.util.concurrent.ThreadFactory");
		add("javax.jms.XAConnectionFactory");
		add("javax.sql.DataSource");
		add("javax.sql.XADataSource");
		add("org.apache.tomcat.jdbc.pool.PoolConfiguration");
		add("org.apache.tomcat.jdbc.pool.Validator");
		add("org.flywaydb.core.api.callback.FlywayCallback");
		add("org.flywaydb.core.api.resolver.MigrationResolver");
	}

	private void add(String className) {
		this.excludes.add(className);
	}

	public boolean isExcluded(TypeMirror type) {
		if (type == null) {
			return false;
		}
		String typeName = type.toString();
		if (typeName.endsWith("[]")) {
			typeName = typeName.substring(0, typeName.length() - 2);
		}
		return this.excludes.contains(typeName);
	}

}
