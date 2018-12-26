

package org.springframework.boot.devtools.restart;

import java.lang.reflect.Method;
import java.lang.reflect.Modifier;

import org.springframework.util.Assert;


class MainMethod {

	private final Method method;

	MainMethod() {
		this(Thread.currentThread());
	}

	MainMethod(Thread thread) {
		Assert.notNull(thread, "Thread must not be null");
		this.method = getMainMethod(thread);
	}

	private Method getMainMethod(Thread thread) {
		for (StackTraceElement element : thread.getStackTrace()) {
			if ("main".equals(element.getMethodName())) {
				Method method = getMainMethod(element);
				if (method != null) {
					return method;
				}
			}
		}
		throw new IllegalStateException("Unable to find main method");
	}

	private Method getMainMethod(StackTraceElement element) {
		try {
			Class<?> elementClass = Class.forName(element.getClassName());
			Method method = elementClass.getDeclaredMethod("main", String[].class);
			if (Modifier.isStatic(method.getModifiers())) {
				return method;
			}
		}
		catch (Exception ex) {
					}
		return null;
	}

	
	public Method getMethod() {
		return this.method;
	}

	
	public String getDeclaringClassName() {
		return this.method.getDeclaringClass().getName();
	}

}
