

package org.springframework.boot.configurationprocessor.fieldvalues.javac;

import java.lang.reflect.Method;


class ReflectionWrapper {

	private final Class<?> type;

	private final Object instance;

	ReflectionWrapper(String type, Object instance) {
		this.type = findClass(instance.getClass().getClassLoader(), type);
		this.instance = this.type.cast(instance);
	}

	protected final Object getInstance() {
		return this.instance;
	}

	@Override
	public String toString() {
		return this.instance.toString();
	}

	protected Class<?> findClass(String name) {
		return findClass(getInstance().getClass().getClassLoader(), name);
	}

	protected Method findMethod(String name, Class<?>... parameterTypes) {
		return findMethod(this.type, name, parameterTypes);
	}

	protected static Class<?> findClass(ClassLoader classLoader, String name) {
		try {
			return classLoader.loadClass(name);
		}
		catch (ClassNotFoundException ex) {
			throw new IllegalStateException(ex);
		}
	}

	protected static Method findMethod(Class<?> type, String name,
			Class<?>... parameterTypes) {
		try {
			return type.getMethod(name, parameterTypes);
		}
		catch (Exception ex) {
			throw new IllegalStateException(ex);
		}
	}

}
