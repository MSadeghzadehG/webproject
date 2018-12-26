

package org.springframework.boot.testsupport.context;

import java.io.File;
import java.io.IOException;
import java.util.HashSet;
import java.util.Set;

import org.junit.Test;

import org.springframework.asm.Opcodes;
import org.springframework.beans.DirectFieldAccessor;
import org.springframework.context.annotation.Bean;
import org.springframework.context.annotation.Configuration;
import org.springframework.core.io.Resource;
import org.springframework.core.io.support.PathMatchingResourcePatternResolver;
import org.springframework.core.io.support.ResourcePatternResolver;
import org.springframework.core.type.AnnotationMetadata;
import org.springframework.core.type.MethodMetadata;
import org.springframework.core.type.classreading.MetadataReader;
import org.springframework.core.type.classreading.SimpleMetadataReaderFactory;

import static org.assertj.core.api.Assertions.assertThat;


public abstract class AbstractConfigurationClassTests {

	private ResourcePatternResolver resolver = new PathMatchingResourcePatternResolver();

	@Test
	public void allBeanMethodsArePublic() throws IOException {
		Set<String> nonPublicBeanMethods = new HashSet<>();
		for (AnnotationMetadata configurationClass : findConfigurationClasses()) {
			Set<MethodMetadata> beanMethods = configurationClass
					.getAnnotatedMethods(Bean.class.getName());
			for (MethodMetadata methodMetadata : beanMethods) {
				if (!isPublic(methodMetadata)) {
					nonPublicBeanMethods.add(methodMetadata.getDeclaringClassName() + "."
							+ methodMetadata.getMethodName());
				}
			}
		}
		assertThat(nonPublicBeanMethods).as("Found non-public @Bean methods").isEmpty();
	}

	private Set<AnnotationMetadata> findConfigurationClasses() throws IOException {
		Set<AnnotationMetadata> configurationClasses = new HashSet<>();
		Resource[] resources = this.resolver.getResources("classpath*:"
				+ getClass().getPackage().getName().replace('.', '/') + "*.class");
		for (Resource resource : resources) {
			if (!isTestClass(resource)) {
				MetadataReader metadataReader = new SimpleMetadataReaderFactory()
						.getMetadataReader(resource);
				AnnotationMetadata annotationMetadata = metadataReader
						.getAnnotationMetadata();
				if (annotationMetadata.getAnnotationTypes()
						.contains(Configuration.class.getName())) {
					configurationClasses.add(annotationMetadata);
				}
			}
		}
		return configurationClasses;
	}

	private boolean isTestClass(Resource resource) throws IOException {
		return resource.getFile().getAbsolutePath()
				.contains("target" + File.separator + "test-classes");
	}

	private boolean isPublic(MethodMetadata methodMetadata) {
		int access = (Integer) new DirectFieldAccessor(methodMetadata)
				.getPropertyValue("access");
		return (access & Opcodes.ACC_PUBLIC) != 0;
	}

}
