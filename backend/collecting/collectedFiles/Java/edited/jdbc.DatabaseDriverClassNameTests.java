

package org.springframework.boot.jdbc;

import java.io.IOException;
import java.sql.Driver;
import java.util.ArrayList;
import java.util.Collections;
import java.util.EnumSet;
import java.util.List;
import java.util.Set;

import javax.sql.XADataSource;

import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.Parameterized;
import org.junit.runners.Parameterized.Parameters;

import org.springframework.asm.ClassReader;

import static org.assertj.core.api.Assertions.assertThat;


@RunWith(Parameterized.class)
public class DatabaseDriverClassNameTests {

	private static final Set<DatabaseDriver> EXCLUDED_DRIVERS = Collections
			.unmodifiableSet(EnumSet.of(DatabaseDriver.UNKNOWN, DatabaseDriver.ORACLE,
					DatabaseDriver.DB2, DatabaseDriver.DB2_AS400, DatabaseDriver.INFORMIX,
					DatabaseDriver.TERADATA));

	private final String className;

	private final Class<?> requiredType;

	@Parameters(name = "{0} {2}")
	public static List<Object[]> parameters() {
		DatabaseDriver[] databaseDrivers = DatabaseDriver.values();
		List<Object[]> parameters = new ArrayList<>();
		for (DatabaseDriver databaseDriver : databaseDrivers) {
			if (EXCLUDED_DRIVERS.contains(databaseDriver)) {
				continue;
			}
			parameters.add(new Object[] { databaseDriver,
					databaseDriver.getDriverClassName(), Driver.class });
			if (databaseDriver.getXaDataSourceClassName() != null) {
				parameters.add(new Object[] { databaseDriver,
						databaseDriver.getXaDataSourceClassName(), XADataSource.class });
			}
		}
		return parameters;
	}

	public DatabaseDriverClassNameTests(DatabaseDriver driver, String className,
			Class<?> requiredType) {
		this.className = className;
		this.requiredType = requiredType;
	}

	@Test
	public void databaseClassIsOfRequiredType() throws Exception {
		assertThat(getInterfaceNames(this.className.replace('.', '/')))
				.contains(this.requiredType.getName().replace('.', '/'));
	}

	private List<String> getInterfaceNames(String className) throws IOException {
				ClassReader classReader = new ClassReader(
				getClass().getResourceAsStream("/" + className + ".class"));
		List<String> interfaceNames = new ArrayList<>();
		for (String name : classReader.getInterfaces()) {
			interfaceNames.add(name);
			interfaceNames.addAll(getInterfaceNames(name));
		}
		return interfaceNames;
	}

}
