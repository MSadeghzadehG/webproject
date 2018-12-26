

package org.springframework.boot.test.context;

import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.util.Collections;
import java.util.Set;

import kotlin.Metadata;
import org.junit.Test;
import org.spockframework.runtime.model.SpecMetadata;
import spock.lang.Issue;
import spock.lang.Stepwise;

import org.springframework.boot.context.annotation.DeterminableImports;
import org.springframework.context.annotation.Configuration;
import org.springframework.context.annotation.Import;
import org.springframework.context.annotation.ImportSelector;
import org.springframework.core.type.AnnotationMetadata;

import static org.assertj.core.api.Assertions.assertThat;


public class ImportsContextCustomizerTests {

	@Test
	public void importSelectorsCouldUseAnyAnnotations() {
		assertThat(new ImportsContextCustomizer(FirstImportSelectorAnnotatedClass.class))
				.isNotEqualTo(new ImportsContextCustomizer(
						SecondImportSelectorAnnotatedClass.class));
	}

	@Test
	public void determinableImportSelector() {
		assertThat(new ImportsContextCustomizer(
				FirstDeterminableImportSelectorAnnotatedClass.class))
						.isEqualTo(new ImportsContextCustomizer(
								SecondDeterminableImportSelectorAnnotatedClass.class));
	}

	@Test
	public void customizersForTestClassesWithDifferentKotlinMetadataAreEqual() {
		assertThat(new ImportsContextCustomizer(FirstKotlinAnnotatedTestClass.class))
				.isEqualTo(new ImportsContextCustomizer(
						SecondKotlinAnnotatedTestClass.class));
	}

	@Test
	public void customizersForTestClassesWithDifferentSpockFrameworkAnnotationsAreEqual() {
		assertThat(
				new ImportsContextCustomizer(FirstSpockFrameworkAnnotatedTestClass.class))
						.isEqualTo(new ImportsContextCustomizer(
								SecondSpockFrameworkAnnotatedTestClass.class));
	}

	@Test
	public void customizersForTestClassesWithDifferentSpockLangAnnotationsAreEqual() {
		assertThat(new ImportsContextCustomizer(FirstSpockLangAnnotatedTestClass.class))
				.isEqualTo(new ImportsContextCustomizer(
						SecondSpockLangAnnotatedTestClass.class));
	}

	@Import(TestImportSelector.class)
	@Indicator1
	static class FirstImportSelectorAnnotatedClass {

	}

	@Import(TestImportSelector.class)
	@Indicator2
	static class SecondImportSelectorAnnotatedClass {

	}

	@Import(TestDeterminableImportSelector.class)
	@Indicator1
	static class FirstDeterminableImportSelectorAnnotatedClass {

	}

	@Import(TestDeterminableImportSelector.class)
	@Indicator2
	static class SecondDeterminableImportSelectorAnnotatedClass {

	}

	@Metadata(d2 = "foo")
	static class FirstKotlinAnnotatedTestClass {

	}

	@Metadata(d2 = "bar")
	static class SecondKotlinAnnotatedTestClass {

	}

	@SpecMetadata(filename = "foo", line = 10)
	static class FirstSpockFrameworkAnnotatedTestClass {

	}

	@SpecMetadata(filename = "bar", line = 10)
	static class SecondSpockFrameworkAnnotatedTestClass {

	}

	@Stepwise
	static class FirstSpockLangAnnotatedTestClass {

	}

	@Issue("1234")
	static class SecondSpockLangAnnotatedTestClass {

	}

	@Retention(RetentionPolicy.RUNTIME)
	@interface Indicator1 {

	}

	@Retention(RetentionPolicy.RUNTIME)
	@interface Indicator2 {

	}

	static class TestImportSelector implements ImportSelector {

		@Override
		public String[] selectImports(AnnotationMetadata arg0) {
			return new String[] {};
		}

	}

	static class TestDeterminableImportSelector
			implements ImportSelector, DeterminableImports {

		@Override
		public String[] selectImports(AnnotationMetadata arg0) {
			return new String[] { TestConfig.class.getName() };
		}

		@Override
		public Set<Object> determineImports(AnnotationMetadata metadata) {
			return Collections.singleton(TestConfig.class.getName());
		}

	}

	@Configuration
	static class TestConfig {

	}

}
