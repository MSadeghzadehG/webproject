

package org.springframework.boot.test.context;

import java.lang.annotation.Annotation;
import java.lang.reflect.AnnotatedElement;
import java.lang.reflect.Constructor;
import java.util.Collections;
import java.util.HashSet;
import java.util.LinkedHashSet;
import java.util.Set;

import org.springframework.beans.BeansException;
import org.springframework.beans.factory.BeanFactory;
import org.springframework.beans.factory.BeanFactoryAware;
import org.springframework.beans.factory.NoSuchBeanDefinitionException;
import org.springframework.beans.factory.config.BeanDefinition;
import org.springframework.beans.factory.config.ConfigurableListableBeanFactory;
import org.springframework.beans.factory.support.BeanDefinitionRegistry;
import org.springframework.beans.factory.support.BeanDefinitionRegistryPostProcessor;
import org.springframework.boot.context.annotation.DeterminableImports;
import org.springframework.context.ApplicationContext;
import org.springframework.context.ConfigurableApplicationContext;
import org.springframework.context.annotation.AnnotatedBeanDefinitionReader;
import org.springframework.context.annotation.Configuration;
import org.springframework.context.annotation.Import;
import org.springframework.context.annotation.ImportBeanDefinitionRegistrar;
import org.springframework.context.annotation.ImportSelector;
import org.springframework.context.support.AbstractApplicationContext;
import org.springframework.core.Ordered;
import org.springframework.core.annotation.AnnotationUtils;
import org.springframework.core.annotation.Order;
import org.springframework.core.style.ToStringCreator;
import org.springframework.core.type.AnnotationMetadata;
import org.springframework.core.type.StandardAnnotationMetadata;
import org.springframework.test.context.ContextCustomizer;
import org.springframework.test.context.MergedContextConfiguration;
import org.springframework.util.ReflectionUtils;


class ImportsContextCustomizer implements ContextCustomizer {

	static final String TEST_CLASS_ATTRIBUTE = "testClass";

	private final Class<?> testClass;

	private final ContextCustomizerKey key;

	ImportsContextCustomizer(Class<?> testClass) {
		this.testClass = testClass;
		this.key = new ContextCustomizerKey(testClass);
	}

	@Override
	public void customizeContext(ConfigurableApplicationContext context,
			MergedContextConfiguration mergedContextConfiguration) {
		BeanDefinitionRegistry registry = getBeanDefinitionRegistry(context);
		AnnotatedBeanDefinitionReader reader = new AnnotatedBeanDefinitionReader(
				registry);
		registerCleanupPostProcessor(registry, reader);
		registerImportsConfiguration(registry, reader);
	}

	private void registerCleanupPostProcessor(BeanDefinitionRegistry registry,
			AnnotatedBeanDefinitionReader reader) {
		BeanDefinition definition = registerBean(registry, reader,
				ImportsCleanupPostProcessor.BEAN_NAME, ImportsCleanupPostProcessor.class);
		definition.getConstructorArgumentValues().addIndexedArgumentValue(0,
				this.testClass);
	}

	private void registerImportsConfiguration(BeanDefinitionRegistry registry,
			AnnotatedBeanDefinitionReader reader) {
		BeanDefinition definition = registerBean(registry, reader,
				ImportsConfiguration.BEAN_NAME, ImportsConfiguration.class);
		definition.setAttribute(TEST_CLASS_ATTRIBUTE, this.testClass);
	}

	private BeanDefinitionRegistry getBeanDefinitionRegistry(ApplicationContext context) {
		if (context instanceof BeanDefinitionRegistry) {
			return (BeanDefinitionRegistry) context;
		}
		if (context instanceof AbstractApplicationContext) {
			return (BeanDefinitionRegistry) ((AbstractApplicationContext) context)
					.getBeanFactory();
		}
		throw new IllegalStateException("Could not locate BeanDefinitionRegistry");
	}

	@SuppressWarnings("unchecked")
	private BeanDefinition registerBean(BeanDefinitionRegistry registry,
			AnnotatedBeanDefinitionReader reader, String beanName, Class<?> type) {
		reader.registerBean(type, beanName);
		return registry.getBeanDefinition(beanName);
	}

	@Override
	public int hashCode() {
		return this.key.hashCode();
	}

	@Override
	public boolean equals(Object obj) {
		if (obj == this) {
			return true;
		}
		if (obj == null || obj.getClass() != getClass()) {
			return false;
		}
				ImportsContextCustomizer other = (ImportsContextCustomizer) obj;
		return this.key.equals(other.key);
	}

	@Override
	public String toString() {
		return new ToStringCreator(this).append("key", this.key).toString();
	}

	
	@Configuration
	@Import(ImportsSelector.class)
	static class ImportsConfiguration {

		static final String BEAN_NAME = ImportsConfiguration.class.getName();

	}

	
	static class ImportsSelector implements ImportSelector, BeanFactoryAware {

		private static final String[] NO_IMPORTS = {};

		private ConfigurableListableBeanFactory beanFactory;

		@Override
		public void setBeanFactory(BeanFactory beanFactory) throws BeansException {
			this.beanFactory = (ConfigurableListableBeanFactory) beanFactory;
		}

		@Override
		public String[] selectImports(AnnotationMetadata importingClassMetadata) {
			BeanDefinition definition = this.beanFactory
					.getBeanDefinition(ImportsConfiguration.BEAN_NAME);
			Object testClass = (definition == null ? null
					: definition.getAttribute(TEST_CLASS_ATTRIBUTE));
			return (testClass == null ? NO_IMPORTS
					: new String[] { ((Class<?>) testClass).getName() });
		}

	}

	
	@Order(Ordered.LOWEST_PRECEDENCE)
	static class ImportsCleanupPostProcessor
			implements BeanDefinitionRegistryPostProcessor {

		static final String BEAN_NAME = ImportsCleanupPostProcessor.class.getName();

		private final Class<?> testClass;

		ImportsCleanupPostProcessor(Class<?> testClass) {
			this.testClass = testClass;
		}

		@Override
		public void postProcessBeanFactory(ConfigurableListableBeanFactory beanFactory)
				throws BeansException {
		}

		@Override
		public void postProcessBeanDefinitionRegistry(BeanDefinitionRegistry registry)
				throws BeansException {
			try {
				String[] names = registry.getBeanDefinitionNames();
				for (String name : names) {
					BeanDefinition definition = registry.getBeanDefinition(name);
					if (this.testClass.getName().equals(definition.getBeanClassName())) {
						registry.removeBeanDefinition(name);
					}
				}
				registry.removeBeanDefinition(ImportsConfiguration.BEAN_NAME);
			}
			catch (NoSuchBeanDefinitionException ex) {
			}
		}

	}

	
	static class ContextCustomizerKey {

		private static final Class<?>[] NO_IMPORTS = {};

		private static final Set<AnnotationFilter> ANNOTATION_FILTERS;

		static {
			Set<AnnotationFilter> filters = new HashSet<>();
			filters.add(new JavaLangAnnotationFilter());
			filters.add(new KotlinAnnotationFilter());
			filters.add(new SpockAnnotationFilter());
			ANNOTATION_FILTERS = Collections.unmodifiableSet(filters);
		}

		private final Set<Object> key;

		ContextCustomizerKey(Class<?> testClass) {
			Set<Annotation> annotations = new HashSet<>();
			Set<Class<?>> seen = new HashSet<>();
			collectClassAnnotations(testClass, annotations, seen);
			Set<Object> determinedImports = determineImports(annotations, testClass);
			this.key = Collections.unmodifiableSet(
					determinedImports != null ? determinedImports : annotations);
		}

		private void collectClassAnnotations(Class<?> classType,
				Set<Annotation> annotations, Set<Class<?>> seen) {
			if (seen.add(classType)) {
				collectElementAnnotations(classType, annotations, seen);
				for (Class<?> interfaceType : classType.getInterfaces()) {
					collectClassAnnotations(interfaceType, annotations, seen);
				}
				if (classType.getSuperclass() != null) {
					collectClassAnnotations(classType.getSuperclass(), annotations, seen);
				}
			}
		}

		private void collectElementAnnotations(AnnotatedElement element,
				Set<Annotation> annotations, Set<Class<?>> seen) {
			for (Annotation annotation : element.getDeclaredAnnotations()) {
				if (!isIgnoredAnnotation(annotation)) {
					annotations.add(annotation);
					collectClassAnnotations(annotation.annotationType(), annotations,
							seen);
				}
			}
		}

		private boolean isIgnoredAnnotation(Annotation annotation) {
			for (AnnotationFilter annotationFilter : ANNOTATION_FILTERS) {
				if (annotationFilter.isIgnored(annotation)) {
					return true;
				}
			}
			return false;
		}

		private Set<Object> determineImports(Set<Annotation> annotations,
				Class<?> testClass) {
			Set<Object> determinedImports = new LinkedHashSet<>();
			AnnotationMetadata testClassMetadata = new StandardAnnotationMetadata(
					testClass);
			for (Annotation annotation : annotations) {
				for (Class<?> source : getImports(annotation)) {
					Set<Object> determinedSourceImports = determineImports(source,
							testClassMetadata);
					if (determinedSourceImports == null) {
						return null;
					}
					determinedImports.addAll(determinedSourceImports);
				}
			}
			return determinedImports;
		}

		private Class<?>[] getImports(Annotation annotation) {
			if (annotation instanceof Import) {
				return ((Import) annotation).value();
			}
			return NO_IMPORTS;
		}

		private Set<Object> determineImports(Class<?> source,
				AnnotationMetadata metadata) {
			if (DeterminableImports.class.isAssignableFrom(source)) {
								return ((DeterminableImports) instantiate(source))
						.determineImports(metadata);
			}
			if (ImportSelector.class.isAssignableFrom(source)
					|| ImportBeanDefinitionRegistrar.class.isAssignableFrom(source)) {
												return null;
			}
						return Collections.singleton(source.getName());
		}

		@SuppressWarnings("unchecked")
		private <T> T instantiate(Class<T> source) {
			try {
				Constructor<?> constructor = source.getDeclaredConstructor();
				ReflectionUtils.makeAccessible(constructor);
				return (T) constructor.newInstance();
			}
			catch (Throwable ex) {
				throw new IllegalStateException(
						"Unable to instantiate DeterminableImportSelector "
								+ source.getName(),
						ex);
			}
		}

		@Override
		public int hashCode() {
			return this.key.hashCode();
		}

		@Override
		public boolean equals(Object obj) {
			return (obj != null && getClass() == obj.getClass()
					&& this.key.equals(((ContextCustomizerKey) obj).key));
		}

		@Override
		public String toString() {
			return this.key.toString();
		}
	}

	
	private interface AnnotationFilter {

		boolean isIgnored(Annotation annotation);

	}

	
	private static final class JavaLangAnnotationFilter implements AnnotationFilter {

		@Override
		public boolean isIgnored(Annotation annotation) {
			return AnnotationUtils.isInJavaLangAnnotationPackage(annotation);
		}

	}

	
	private static final class KotlinAnnotationFilter implements AnnotationFilter {

		@Override
		public boolean isIgnored(Annotation annotation) {
			return "kotlin.Metadata".equals(annotation.annotationType().getName())
					|| isInKotlinAnnotationPackage(annotation);
		}

		private boolean isInKotlinAnnotationPackage(Annotation annotation) {
			return annotation.annotationType().getName().startsWith("kotlin.annotation.");
		}

	}

	
	private static final class SpockAnnotationFilter implements AnnotationFilter {

		@Override
		public boolean isIgnored(Annotation annotation) {
			return annotation.annotationType().getName().startsWith("org.spockframework.")
					|| annotation.annotationType().getName().startsWith("spock.");
		}

	}

}
