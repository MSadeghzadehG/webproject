

package org.springframework.boot.web.servlet;

import java.util.AbstractCollection;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.EventListener;
import java.util.HashSet;
import java.util.Iterator;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;
import java.util.Map.Entry;
import java.util.Set;

import javax.servlet.Filter;
import javax.servlet.MultipartConfigElement;
import javax.servlet.Servlet;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import org.springframework.aop.scope.ScopedProxyUtils;
import org.springframework.beans.factory.ListableBeanFactory;
import org.springframework.beans.factory.support.BeanDefinitionRegistry;
import org.springframework.core.annotation.AnnotationAwareOrderComparator;
import org.springframework.util.LinkedMultiValueMap;
import org.springframework.util.MultiValueMap;


public class ServletContextInitializerBeans
		extends AbstractCollection<ServletContextInitializer> {

	private static final String DISPATCHER_SERVLET_NAME = "dispatcherServlet";

	private static final Log logger = LogFactory
			.getLog(ServletContextInitializerBeans.class);

	
	private final Set<Object> seen = new HashSet<>();

	private final MultiValueMap<Class<?>, ServletContextInitializer> initializers;

	private List<ServletContextInitializer> sortedList;

	public ServletContextInitializerBeans(ListableBeanFactory beanFactory) {
		this.initializers = new LinkedMultiValueMap<>();
		addServletContextInitializerBeans(beanFactory);
		addAdaptableBeans(beanFactory);
		List<ServletContextInitializer> sortedInitializers = new ArrayList<>();
		for (Map.Entry<?, List<ServletContextInitializer>> entry : this.initializers
				.entrySet()) {
			AnnotationAwareOrderComparator.sort(entry.getValue());
			sortedInitializers.addAll(entry.getValue());
		}
		this.sortedList = Collections.unmodifiableList(sortedInitializers);
	}

	private void addServletContextInitializerBeans(ListableBeanFactory beanFactory) {
		for (Entry<String, ServletContextInitializer> initializerBean : getOrderedBeansOfType(
				beanFactory, ServletContextInitializer.class)) {
			addServletContextInitializerBean(initializerBean.getKey(),
					initializerBean.getValue(), beanFactory);
		}
	}

	private void addServletContextInitializerBean(String beanName,
			ServletContextInitializer initializer, ListableBeanFactory beanFactory) {
		if (initializer instanceof ServletRegistrationBean) {
			Servlet source = ((ServletRegistrationBean<?>) initializer).getServlet();
			addServletContextInitializerBean(Servlet.class, beanName, initializer,
					beanFactory, source);
		}
		else if (initializer instanceof FilterRegistrationBean) {
			Filter source = ((FilterRegistrationBean<?>) initializer).getFilter();
			addServletContextInitializerBean(Filter.class, beanName, initializer,
					beanFactory, source);
		}
		else if (initializer instanceof DelegatingFilterProxyRegistrationBean) {
			String source = ((DelegatingFilterProxyRegistrationBean) initializer)
					.getTargetBeanName();
			addServletContextInitializerBean(Filter.class, beanName, initializer,
					beanFactory, source);
		}
		else if (initializer instanceof ServletListenerRegistrationBean) {
			EventListener source = ((ServletListenerRegistrationBean<?>) initializer)
					.getListener();
			addServletContextInitializerBean(EventListener.class, beanName, initializer,
					beanFactory, source);
		}
		else {
			addServletContextInitializerBean(ServletContextInitializer.class, beanName,
					initializer, beanFactory, initializer);
		}
	}

	private void addServletContextInitializerBean(Class<?> type, String beanName,
			ServletContextInitializer initializer, ListableBeanFactory beanFactory,
			Object source) {
		this.initializers.add(type, initializer);
		if (source != null) {
						this.seen.add(source);
		}
		if (ServletContextInitializerBeans.logger.isDebugEnabled()) {
			String resourceDescription = getResourceDescription(beanName, beanFactory);
			int order = getOrder(initializer);
			ServletContextInitializerBeans.logger.debug("Added existing "
					+ type.getSimpleName() + " initializer bean '" + beanName
					+ "'; order=" + order + ", resource=" + resourceDescription);
		}
	}

	private String getResourceDescription(String beanName,
			ListableBeanFactory beanFactory) {
		if (beanFactory instanceof BeanDefinitionRegistry) {
			BeanDefinitionRegistry registry = (BeanDefinitionRegistry) beanFactory;
			return registry.getBeanDefinition(beanName).getResourceDescription();
		}
		return "unknown";
	}

	@SuppressWarnings("unchecked")
	private void addAdaptableBeans(ListableBeanFactory beanFactory) {
		MultipartConfigElement multipartConfig = getMultipartConfig(beanFactory);
		addAsRegistrationBean(beanFactory, Servlet.class,
				new ServletRegistrationBeanAdapter(multipartConfig));
		addAsRegistrationBean(beanFactory, Filter.class,
				new FilterRegistrationBeanAdapter());
		for (Class<?> listenerType : ServletListenerRegistrationBean
				.getSupportedTypes()) {
			addAsRegistrationBean(beanFactory, EventListener.class,
					(Class<EventListener>) listenerType,
					new ServletListenerRegistrationBeanAdapter());
		}
	}

	private MultipartConfigElement getMultipartConfig(ListableBeanFactory beanFactory) {
		List<Entry<String, MultipartConfigElement>> beans = getOrderedBeansOfType(
				beanFactory, MultipartConfigElement.class);
		return (beans.isEmpty() ? null : beans.get(0).getValue());
	}

	private <T> void addAsRegistrationBean(ListableBeanFactory beanFactory, Class<T> type,
			RegistrationBeanAdapter<T> adapter) {
		addAsRegistrationBean(beanFactory, type, type, adapter);
	}

	private <T, B extends T> void addAsRegistrationBean(ListableBeanFactory beanFactory,
			Class<T> type, Class<B> beanType, RegistrationBeanAdapter<T> adapter) {
		List<Map.Entry<String, B>> beans = getOrderedBeansOfType(beanFactory, beanType,
				this.seen);
		for (Entry<String, B> bean : beans) {
			if (this.seen.add(bean.getValue())) {
				int order = getOrder(bean.getValue());
				String beanName = bean.getKey();
								RegistrationBean registration = adapter.createRegistrationBean(beanName,
						bean.getValue(), beans.size());
				registration.setOrder(order);
				this.initializers.add(type, registration);
				if (ServletContextInitializerBeans.logger.isDebugEnabled()) {
					ServletContextInitializerBeans.logger.debug(
							"Created " + type.getSimpleName() + " initializer for bean '"
									+ beanName + "'; order=" + order + ", resource="
									+ getResourceDescription(beanName, beanFactory));
				}
			}
		}
	}

	private int getOrder(Object value) {
		return new AnnotationAwareOrderComparator() {
			@Override
			public int getOrder(Object obj) {
				return super.getOrder(obj);
			}
		}.getOrder(value);
	}

	private <T> List<Entry<String, T>> getOrderedBeansOfType(
			ListableBeanFactory beanFactory, Class<T> type) {
		return getOrderedBeansOfType(beanFactory, type, Collections.emptySet());
	}

	private <T> List<Entry<String, T>> getOrderedBeansOfType(
			ListableBeanFactory beanFactory, Class<T> type, Set<?> excludes) {
		Comparator<Entry<String, T>> comparator = (o1,
				o2) -> AnnotationAwareOrderComparator.INSTANCE.compare(o1.getValue(),
						o2.getValue());
		String[] names = beanFactory.getBeanNamesForType(type, true, false);
		Map<String, T> map = new LinkedHashMap<>();
		for (String name : names) {
			if (!excludes.contains(name) && !ScopedProxyUtils.isScopedTarget(name)) {
				T bean = beanFactory.getBean(name, type);
				if (!excludes.contains(bean)) {
					map.put(name, bean);
				}
			}
		}
		List<Entry<String, T>> beans = new ArrayList<>();
		beans.addAll(map.entrySet());
		beans.sort(comparator);
		return beans;
	}

	@Override
	public Iterator<ServletContextInitializer> iterator() {
		return this.sortedList.iterator();
	}

	@Override
	public int size() {
		return this.sortedList.size();
	}

	
	private interface RegistrationBeanAdapter<T> {

		RegistrationBean createRegistrationBean(String name, T source,
				int totalNumberOfSourceBeans);

	}

	
	private static class ServletRegistrationBeanAdapter
			implements RegistrationBeanAdapter<Servlet> {

		private final MultipartConfigElement multipartConfig;

		ServletRegistrationBeanAdapter(MultipartConfigElement multipartConfig) {
			this.multipartConfig = multipartConfig;
		}

		@Override
		public RegistrationBean createRegistrationBean(String name, Servlet source,
				int totalNumberOfSourceBeans) {
			String url = (totalNumberOfSourceBeans == 1 ? "/" : "/" + name + "/");
			if (name.equals(DISPATCHER_SERVLET_NAME)) {
				url = "/"; 			}
			ServletRegistrationBean<Servlet> bean = new ServletRegistrationBean<>(source,
					url);
			bean.setName(name);
			bean.setMultipartConfig(this.multipartConfig);
			return bean;
		}

	}

	
	private static class FilterRegistrationBeanAdapter
			implements RegistrationBeanAdapter<Filter> {

		@Override
		public RegistrationBean createRegistrationBean(String name, Filter source,
				int totalNumberOfSourceBeans) {
			FilterRegistrationBean<Filter> bean = new FilterRegistrationBean<>(source);
			bean.setName(name);
			return bean;
		}

	}

	
	private static class ServletListenerRegistrationBeanAdapter
			implements RegistrationBeanAdapter<EventListener> {

		@Override
		public RegistrationBean createRegistrationBean(String name, EventListener source,
				int totalNumberOfSourceBeans) {
			return new ServletListenerRegistrationBean<>(source);
		}

	}

}
