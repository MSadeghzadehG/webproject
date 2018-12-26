

package org.springframework.boot.cli.command.archive;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;
import java.net.URI;
import java.net.URISyntaxException;
import java.net.URL;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.Locale;
import java.util.jar.Manifest;

import groovy.lang.Grab;
import joptsimple.OptionSet;
import joptsimple.OptionSpec;
import org.codehaus.groovy.ast.ASTNode;
import org.codehaus.groovy.ast.AnnotatedNode;
import org.codehaus.groovy.ast.AnnotationNode;
import org.codehaus.groovy.ast.ClassNode;
import org.codehaus.groovy.ast.ModuleNode;
import org.codehaus.groovy.ast.expr.ConstantExpression;
import org.codehaus.groovy.control.SourceUnit;
import org.codehaus.groovy.transform.ASTTransformation;

import org.springframework.boot.cli.app.SpringApplicationLauncher;
import org.springframework.boot.cli.archive.PackagedSpringApplicationLauncher;
import org.springframework.boot.cli.command.Command;
import org.springframework.boot.cli.command.OptionParsingCommand;
import org.springframework.boot.cli.command.archive.ResourceMatcher.MatchedResource;
import org.springframework.boot.cli.command.options.CompilerOptionHandler;
import org.springframework.boot.cli.command.options.OptionHandler;
import org.springframework.boot.cli.command.options.OptionSetGroovyCompilerConfiguration;
import org.springframework.boot.cli.command.options.SourceOptions;
import org.springframework.boot.cli.command.status.ExitStatus;
import org.springframework.boot.cli.compiler.GroovyCompiler;
import org.springframework.boot.cli.compiler.GroovyCompilerConfiguration;
import org.springframework.boot.cli.compiler.RepositoryConfigurationFactory;
import org.springframework.boot.cli.compiler.grape.RepositoryConfiguration;
import org.springframework.boot.loader.tools.JarWriter;
import org.springframework.boot.loader.tools.Layout;
import org.springframework.boot.loader.tools.Library;
import org.springframework.boot.loader.tools.LibraryScope;
import org.springframework.boot.loader.tools.Repackager;
import org.springframework.core.io.Resource;
import org.springframework.core.io.support.PathMatchingResourcePatternResolver;
import org.springframework.util.Assert;


abstract class ArchiveCommand extends OptionParsingCommand {

	protected ArchiveCommand(String name, String description,
			OptionHandler optionHandler) {
		super(name, description, optionHandler);
	}

	@Override
	public String getUsageHelp() {
		return "[options] <" + getName() + "-name> <files>";
	}

	
	protected abstract static class ArchiveOptionHandler extends CompilerOptionHandler {

		private final String type;

		private final Layout layout;

		private OptionSpec<String> includeOption;

		private OptionSpec<String> excludeOption;

		public ArchiveOptionHandler(String type, Layout layout) {
			this.type = type;
			this.layout = layout;
		}

		protected Layout getLayout() {
			return this.layout;
		}

		@Override
		protected void doOptions() {
			this.includeOption = option("include",
					"Pattern applied to directories on the classpath to find files to "
							+ "include in the resulting ").withRequiredArg()
									.withValuesSeparatedBy(",").defaultsTo("");
			this.excludeOption = option("exclude",
					"Pattern applied to directories on the classpath to find files to "
							+ "exclude from the resulting " + this.type).withRequiredArg()
									.withValuesSeparatedBy(",").defaultsTo("");
		}

		@Override
		protected ExitStatus run(OptionSet options) throws Exception {
			List<?> nonOptionArguments = new ArrayList<Object>(
					options.nonOptionArguments());
			Assert.isTrue(nonOptionArguments.size() >= 2, "The name of the resulting "
					+ this.type + " and at least one source file must be specified");

			File output = new File((String) nonOptionArguments.remove(0));
			Assert.isTrue(
					output.getName().toLowerCase(Locale.ENGLISH)
							.endsWith("." + this.type),
					"The output '" + output + "' is not a "
							+ this.type.toUpperCase(Locale.ENGLISH) + " file.");
			deleteIfExists(output);

			GroovyCompiler compiler = createCompiler(options);

			List<URL> classpath = getClassPathUrls(compiler);
			List<MatchedResource> classpathEntries = findMatchingClasspathEntries(
					classpath, options);

			String[] sources = new SourceOptions(nonOptionArguments).getSourcesArray();
			Class<?>[] compiledClasses = compiler.compile(sources);

			List<URL> dependencies = getClassPathUrls(compiler);
			dependencies.removeAll(classpath);

			writeJar(output, compiledClasses, classpathEntries, dependencies);
			return ExitStatus.OK;
		}

		private void deleteIfExists(File file) {
			if (file.exists() && !file.delete()) {
				throw new IllegalStateException(
						"Failed to delete existing file " + file.getPath());
			}
		}

		private GroovyCompiler createCompiler(OptionSet options) {
			List<RepositoryConfiguration> repositoryConfiguration = RepositoryConfigurationFactory
					.createDefaultRepositoryConfiguration();
			GroovyCompilerConfiguration configuration = new OptionSetGroovyCompilerConfiguration(
					options, this, repositoryConfiguration);
			GroovyCompiler groovyCompiler = new GroovyCompiler(configuration);
			groovyCompiler.getAstTransformations().add(0, new GrabAnnotationTransform());
			return groovyCompiler;
		}

		private List<URL> getClassPathUrls(GroovyCompiler compiler) {
			return new ArrayList<>(Arrays.asList(compiler.getLoader().getURLs()));
		}

		private List<MatchedResource> findMatchingClasspathEntries(List<URL> classpath,
				OptionSet options) throws IOException {
			ResourceMatcher matcher = new ResourceMatcher(
					options.valuesOf(this.includeOption),
					options.valuesOf(this.excludeOption));
			List<File> roots = new ArrayList<>();
			for (URL classpathEntry : classpath) {
				roots.add(new File(URI.create(classpathEntry.toString())));
			}
			return matcher.find(roots);
		}

		private void writeJar(File file, Class<?>[] compiledClasses,
				List<MatchedResource> classpathEntries, List<URL> dependencies)
				throws FileNotFoundException, IOException, URISyntaxException {
			final List<Library> libraries;
			try (JarWriter writer = new JarWriter(file)) {
				addManifest(writer, compiledClasses);
				addCliClasses(writer);
				for (Class<?> compiledClass : compiledClasses) {
					addClass(writer, compiledClass);
				}
				libraries = addClasspathEntries(writer, classpathEntries);
			}
			libraries.addAll(createLibraries(dependencies));
			Repackager repackager = new Repackager(file);
			repackager.setMainClass(PackagedSpringApplicationLauncher.class.getName());
			repackager.repackage((callback) -> {
				for (Library library : libraries) {
					callback.library(library);
				}
			});
		}

		private List<Library> createLibraries(List<URL> dependencies)
				throws URISyntaxException {
			List<Library> libraries = new ArrayList<>();
			for (URL dependency : dependencies) {
				File file = new File(dependency.toURI());
				libraries.add(new Library(file, getLibraryScope(file)));
			}
			return libraries;
		}

		private void addManifest(JarWriter writer, Class<?>[] compiledClasses)
				throws IOException {
			Manifest manifest = new Manifest();
			manifest.getMainAttributes().putValue("Manifest-Version", "1.0");
			manifest.getMainAttributes().putValue(
					PackagedSpringApplicationLauncher.SOURCE_ENTRY,
					commaDelimitedClassNames(compiledClasses));
			writer.writeManifest(manifest);
		}

		private String commaDelimitedClassNames(Class<?>[] classes) {
			StringBuilder builder = new StringBuilder();
			for (int i = 0; i < classes.length; i++) {
				builder.append(i == 0 ? "" : ",");
				builder.append(classes[i].getName());
			}
			return builder.toString();
		}

		protected void addCliClasses(JarWriter writer) throws IOException {
			addClass(writer, PackagedSpringApplicationLauncher.class);
			addClass(writer, SpringApplicationLauncher.class);
			Resource[] resources = new PathMatchingResourcePatternResolver()
					.getResources("org/springframework/boot/groovy
	private static class GrabAnnotationTransform implements ASTTransformation {

		@Override
		public void visit(ASTNode[] nodes, SourceUnit source) {
			for (ASTNode node : nodes) {
				if (node instanceof ModuleNode) {
					visitModule((ModuleNode) node);
				}
			}
		}

		private void visitModule(ModuleNode module) {
			for (ClassNode classNode : module.getClasses()) {
				AnnotationNode annotation = new AnnotationNode(new ClassNode(Grab.class));
				annotation.addMember("value", new ConstantExpression("groovy"));
				classNode.addAnnotation(annotation);
								break;
			}
									disableGrabResolvers(module.getClasses());
			disableGrabResolvers(module.getImports());
		}

		private void disableGrabResolvers(List<? extends AnnotatedNode> nodes) {
			for (AnnotatedNode classNode : nodes) {
				List<AnnotationNode> annotations = classNode.getAnnotations();
				for (AnnotationNode node : new ArrayList<>(annotations)) {
					if (node.getClassNode().getNameWithoutPackage()
							.equals("GrabResolver")) {
						node.setMember("initClass", new ConstantExpression(false));
					}
				}
			}
		}

	}

}
