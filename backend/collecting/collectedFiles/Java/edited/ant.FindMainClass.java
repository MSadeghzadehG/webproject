

package org.springframework.boot.ant;

import java.io.File;
import java.io.IOException;
import java.util.jar.JarFile;

import org.apache.tools.ant.BuildException;
import org.apache.tools.ant.Project;
import org.apache.tools.ant.Task;

import org.springframework.boot.loader.tools.MainClassFinder;
import org.springframework.util.StringUtils;


public class FindMainClass extends Task {

	private static final String SPRING_BOOT_APPLICATION_CLASS_NAME = "org.springframework.boot.autoconfigure.SpringBootApplication";

	private String mainClass;

	private File classesRoot;

	private String property;

	public FindMainClass(Project project) {
		setProject(project);
	}

	@Override
	public void execute() throws BuildException {
		String mainClass = this.mainClass;
		if (!StringUtils.hasText(mainClass)) {
			mainClass = findMainClass();
			if (!StringUtils.hasText(mainClass)) {
				throw new BuildException(
						"Could not determine main class given @classesRoot "
								+ this.classesRoot);
			}
		}
		handle(mainClass);
	}

	private String findMainClass() {
		if (this.classesRoot == null) {
			throw new BuildException(
					"one of @mainClass or @classesRoot must be specified");
		}
		if (!this.classesRoot.exists()) {
			throw new BuildException(
					"@classesRoot " + this.classesRoot + " does not exist");
		}
		try {
			if (this.classesRoot.isDirectory()) {
				return MainClassFinder.findSingleMainClass(this.classesRoot,
						SPRING_BOOT_APPLICATION_CLASS_NAME);
			}
			return MainClassFinder.findSingleMainClass(new JarFile(this.classesRoot), "/",
					SPRING_BOOT_APPLICATION_CLASS_NAME);
		}
		catch (IOException ex) {
			throw new BuildException(ex);
		}
	}

	private void handle(String mainClass) {
		if (StringUtils.hasText(this.property)) {
			getProject().setProperty(this.property, mainClass);
		}
		else {
			log("Found main class " + mainClass);
		}
	}

	
	public void setMainClass(String mainClass) {
		this.mainClass = mainClass;
	}

	
	public void setClassesRoot(File classesRoot) {
		this.classesRoot = classesRoot;
	}

	
	public void setProperty(String property) {
		this.property = property;
	}

}
