

package org.springframework.boot.gradle.plugin;

import java.io.File;
import java.io.IOException;
import java.util.Objects;
import java.util.concurrent.Callable;
import java.util.function.Supplier;

import org.gradle.api.Project;
import org.gradle.api.file.FileCollection;

import org.springframework.boot.gradle.dsl.SpringBootExtension;
import org.springframework.boot.loader.tools.MainClassFinder;


final class MainClassConvention implements Callable<Object> {

	private static final String SPRING_BOOT_APPLICATION_CLASS_NAME = "org.springframework.boot.autoconfigure.SpringBootApplication";

	private final Project project;

	private final Supplier<FileCollection> classpathSupplier;

	MainClassConvention(Project project, Supplier<FileCollection> classpathSupplier) {
		this.project = project;
		this.classpathSupplier = classpathSupplier;
	}

	@Override
	public Object call() throws Exception {
		SpringBootExtension springBootExtension = this.project.getExtensions()
				.findByType(SpringBootExtension.class);
		if (springBootExtension != null
				&& springBootExtension.getMainClassName() != null) {
			return springBootExtension.getMainClassName();
		}
		if (this.project.hasProperty("mainClassName")) {
			Object mainClassName = this.project.property("mainClassName");
			if (mainClassName != null) {
				return mainClassName;
			}
		}
		return resolveMainClass();
	}

	private String resolveMainClass() {
		return this.classpathSupplier.get().filter(File::isDirectory).getFiles().stream()
				.map(this::findMainClass).filter(Objects::nonNull).findFirst()
				.orElse(null);
	}

	private String findMainClass(File file) {
		try {
			return MainClassFinder.findSingleMainClass(file,
					SPRING_BOOT_APPLICATION_CLASS_NAME);
		}
		catch (IOException ex) {
			return null;
		}
	}

}
