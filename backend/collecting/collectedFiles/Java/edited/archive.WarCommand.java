

package org.springframework.boot.cli.command.archive;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;

import org.springframework.boot.cli.command.Command;
import org.springframework.boot.loader.tools.JarWriter;
import org.springframework.boot.loader.tools.Layouts;
import org.springframework.boot.loader.tools.LibraryScope;


public class WarCommand extends ArchiveCommand {

	public WarCommand() {
		super("war", "Create a self-contained executable war "
				+ "file from a Spring Groovy script", new WarOptionHandler());
	}

	private static final class WarOptionHandler extends ArchiveOptionHandler {

		WarOptionHandler() {
			super("war", new Layouts.War());
		}

		@Override
		protected LibraryScope getLibraryScope(File file) {
			String fileName = file.getName();
			if (fileName.contains("tomcat-embed")
					|| fileName.contains("spring-boot-starter-tomcat")) {
				return LibraryScope.PROVIDED;
			}
			return LibraryScope.COMPILE;
		}

		@Override
		protected void addCliClasses(JarWriter writer) throws IOException {
			addClass(writer, null, "org.springframework.boot."
					+ "cli.app.SpringApplicationWebApplicationInitializer");
			super.addCliClasses(writer);
		}

		@Override
		protected void writeClasspathEntry(JarWriter writer,
				ResourceMatcher.MatchedResource entry) throws IOException {
			writer.writeEntry(getLayout().getClassesLocation() + entry.getName(),
					new FileInputStream(entry.getFile()));
		}

	}

}
