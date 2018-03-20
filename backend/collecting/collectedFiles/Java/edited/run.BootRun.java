

package org.springframework.boot.gradle.tasks.run;

import org.gradle.api.file.SourceDirectorySet;
import org.gradle.api.tasks.JavaExec;
import org.gradle.api.tasks.SourceSet;
import org.gradle.api.tasks.SourceSetOutput;


public class BootRun extends JavaExec {

	
	public void sourceResources(SourceSet sourceSet) {
		setClasspath(getProject()
				.files(sourceSet.getResources().getSrcDirs(), getClasspath())
				.filter((file) -> !file.equals(sourceSet.getOutput().getResourcesDir())));
	}

	@Override
	public void exec() {
		if (System.console() != null) {
						this.getEnvironment().put("spring.output.ansi.console-available", true);
		}
		super.exec();
	}

}
