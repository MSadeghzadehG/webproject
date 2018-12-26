

package org.springframework.boot.configurationprocessor;

import java.io.File;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.StringReader;
import java.util.Arrays;
import java.util.HashSet;
import java.util.LinkedHashSet;
import java.util.Set;

import org.junit.Assert;
import org.junit.rules.TemporaryFolder;

import org.springframework.boot.configurationprocessor.metadata.ConfigurationMetadata;
import org.springframework.boot.configurationsample.ConfigurationProperties;
import org.springframework.boot.configurationsample.NestedConfigurationProperty;
import org.springframework.boot.testsupport.compiler.TestCompiler;
import org.springframework.boot.testsupport.compiler.TestCompiler.TestCompilationTask;
import org.springframework.util.FileCopyUtils;
import org.springframework.util.FileSystemUtils;


public class TestProject {

	private static final Class<?>[] ALWAYS_INCLUDE = { ConfigurationProperties.class,
			NestedConfigurationProperty.class };

	
	private File sourceFolder;

	private TestCompiler compiler;

	private Set<File> sourceFiles = new LinkedHashSet<>();

	public TestProject(TemporaryFolder tempFolder, Class<?>... classes)
			throws IOException {
		this.sourceFolder = tempFolder.newFolder();
		this.compiler = new TestCompiler(tempFolder) {
			@Override
			protected File getSourceFolder() {
				return TestProject.this.sourceFolder;
			}
		};
		Set<Class<?>> contents = new HashSet<>(Arrays.asList(classes));
		contents.addAll(Arrays.asList(ALWAYS_INCLUDE));
		copySources(contents);
	}

	private void copySources(Set<Class<?>> contents) throws IOException {
		for (Class<?> type : contents) {
			copySources(type);
		}
	}

	private void copySources(Class<?> type) throws IOException {
		File original = getOriginalSourceFile(type);
		File target = getSourceFile(type);
		target.getParentFile().mkdirs();
		FileCopyUtils.copy(original, target);
		this.sourceFiles.add(target);
	}

	public File getSourceFile(Class<?> type) {
		return new File(this.sourceFolder, TestCompiler.sourcePathFor(type));
	}

	public ConfigurationMetadata fullBuild() {
		TestConfigurationMetadataAnnotationProcessor processor = new TestConfigurationMetadataAnnotationProcessor(
				this.compiler.getOutputLocation());
		TestCompilationTask task = this.compiler.getTask(this.sourceFiles);
		deleteFolderContents(this.compiler.getOutputLocation());
		task.call(processor);
		return processor.getMetadata();
	}

	public ConfigurationMetadata incrementalBuild(Class<?>... toRecompile) {
		TestConfigurationMetadataAnnotationProcessor processor = new TestConfigurationMetadataAnnotationProcessor(
				this.compiler.getOutputLocation());
		TestCompilationTask task = this.compiler.getTask(toRecompile);
		task.call(processor);
		return processor.getMetadata();
	}

	private void deleteFolderContents(File outputFolder) {
		FileSystemUtils.deleteRecursively(outputFolder);
		outputFolder.mkdirs();
	}

	
	public File getOutputFile(String relativePath) {
		Assert.assertFalse(new File(relativePath).isAbsolute());
		return new File(this.compiler.getOutputLocation(), relativePath);
	}

	
	public void addSourceCode(Class<?> target, InputStream snippetStream)
			throws Exception {
		File targetFile = getSourceFile(target);
		String contents = getContents(targetFile);
		int insertAt = contents.lastIndexOf('}');
		String additionalSource = FileCopyUtils
				.copyToString(new InputStreamReader(snippetStream));
		contents = contents.substring(0, insertAt) + additionalSource
				+ contents.substring(insertAt);
		putContents(targetFile, contents);
	}

	
	public void delete(Class<?> type) {
		File target = getSourceFile(type);
		target.delete();
		this.sourceFiles.remove(target);
	}

	
	public void revert(Class<?> type) throws IOException {
		Assert.assertTrue(getSourceFile(type).exists());
		copySources(type);
	}

	
	public void add(Class<?> type) throws IOException {
		Assert.assertFalse(getSourceFile(type).exists());
		copySources(type);
	}

	public void replaceText(Class<?> type, String find, String replace) throws Exception {
		File target = getSourceFile(type);
		String contents = getContents(target);
		contents = contents.replace(find, replace);
		putContents(target, contents);
	}

	
	private File getOriginalSourceFile(Class<?> type) {
		return new File(TestCompiler.SOURCE_FOLDER, TestCompiler.sourcePathFor(type));
	}

	private static void putContents(File targetFile, String contents) throws IOException {
		FileCopyUtils.copy(new StringReader(contents), new FileWriter(targetFile));
	}

	private static String getContents(File file) throws Exception {
		return FileCopyUtils.copyToString(new FileReader(file));
	}

}
