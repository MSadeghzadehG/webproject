

package org.springframework.boot.context.config;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

import org.springframework.beans.BeanUtils;
import org.springframework.boot.context.event.ApplicationEnvironmentPreparedEvent;
import org.springframework.context.ApplicationContextException;
import org.springframework.context.ApplicationEvent;
import org.springframework.context.ApplicationListener;
import org.springframework.context.event.SimpleApplicationEventMulticaster;
import org.springframework.core.Ordered;
import org.springframework.core.annotation.AnnotationAwareOrderComparator;
import org.springframework.core.env.ConfigurableEnvironment;
import org.springframework.util.Assert;
import org.springframework.util.ClassUtils;
import org.springframework.util.StringUtils;


public class DelegatingApplicationListener
		implements ApplicationListener<ApplicationEvent>, Ordered {

	
	private static final String PROPERTY_NAME = "context.listener.classes";

	private int order = 0;

	private SimpleApplicationEventMulticaster multicaster;

	@Override
	public void onApplicationEvent(ApplicationEvent event) {
		if (event instanceof ApplicationEnvironmentPreparedEvent) {
			List<ApplicationListener<ApplicationEvent>> delegates = getListeners(
					((ApplicationEnvironmentPreparedEvent) event).getEnvironment());
			if (delegates.isEmpty()) {
				return;
			}
			this.multicaster = new SimpleApplicationEventMulticaster();
			for (ApplicationListener<ApplicationEvent> listener : delegates) {
				this.multicaster.addApplicationListener(listener);
			}
		}
		if (this.multicaster != null) {
			this.multicaster.multicastEvent(event);
		}
	}

	@SuppressWarnings("unchecked")
	private List<ApplicationListener<ApplicationEvent>> getListeners(
			ConfigurableEnvironment environment) {
		if (environment == null) {
			return Collections.emptyList();
		}
		String classNames = environment.getProperty(PROPERTY_NAME);
		List<ApplicationListener<ApplicationEvent>> listeners = new ArrayList<>();
		if (StringUtils.hasLength(classNames)) {
			for (String className : StringUtils.commaDelimitedListToSet(classNames)) {
				try {
					Class<?> clazz = ClassUtils.forName(className,
							ClassUtils.getDefaultClassLoader());
					Assert.isAssignable(ApplicationListener.class, clazz, "class ["
							+ className + "] must implement ApplicationListener");
					listeners.add((ApplicationListener<ApplicationEvent>) BeanUtils
							.instantiateClass(clazz));
				}
				catch (Exception ex) {
					throw new ApplicationContextException(
							"Failed to load context listener class [" + className + "]",
							ex);
				}
			}
		}
		AnnotationAwareOrderComparator.sort(listeners);
		return listeners;
	}

	public void setOrder(int order) {
		this.order = order;
	}

	@Override
	public int getOrder() {
		return this.order;
	}

}
