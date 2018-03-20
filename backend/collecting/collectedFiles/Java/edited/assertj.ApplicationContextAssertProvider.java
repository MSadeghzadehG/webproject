

package org.springframework.boot.test.context.assertj;

import java.io.Closeable;
import java.lang.reflect.Proxy;
import java.util.function.Supplier;

import org.assertj.core.api.AssertProvider;

import org.springframework.context.ApplicationContext;
import org.springframework.util.Assert;


public interface ApplicationContextAssertProvider<C extends ApplicationContext> extends
		ApplicationContext, AssertProvider<ApplicationContextAssert<C>>, Closeable {

	
	@Deprecated
	@Override
	ApplicationContextAssert<C> assertThat();

	
	C getSourceApplicationContext();

	
	<T extends C> T getSourceApplicationContext(Class<T> requiredType);

	
	Throwable getStartupFailure();

	@Override
	void close();

	
	@SuppressWarnings("unchecked")
	static <T extends ApplicationContextAssertProvider<C>, C extends ApplicationContext> T get(
			Class<T> type, Class<? extends C> contextType,
			Supplier<? extends C> contextSupplier) {
		Assert.notNull(type, "Type must not be null");
		Assert.isTrue(type.isInterface(), "Type must be an interface");
		Assert.notNull(contextType, "ContextType must not be null");
		Assert.isTrue(contextType.isInterface(), "ContextType must be an interface");
		Class<?>[] interfaces = { type, contextType };
		return (T) Proxy.newProxyInstance(Thread.currentThread().getContextClassLoader(),
				interfaces, new AssertProviderApplicationContextInvocationHandler(
						contextType, contextSupplier));
	}

}
