

package org.springframework.boot.loader.tools;

import java.io.File;
import java.util.Collections;
import java.util.HashMap;
import java.util.Locale;
import java.util.Map;


public final class Layouts {

	private Layouts() {
	}

	
	public static Layout forFile(File file) {
		if (file == null) {
			throw new IllegalArgumentException("File must not be null");
		}
		String lowerCaseFileName = file.getName().toLowerCase(Locale.ENGLISH);
		if (lowerCaseFileName.endsWith(".jar")) {
			return new Jar();
		}
		if (lowerCaseFileName.endsWith(".war")) {
			return new War();
		}
		if (file.isDirectory() || lowerCaseFileName.endsWith(".zip")) {
			return new Expanded();
		}
		throw new IllegalStateException("Unable to deduce layout for '" + file + "'");
	}

	
	public static class Jar implements RepackagingLayout {

		@Override
		public String getLauncherClassName() {
			return "org.springframework.boot.loader.JarLauncher";
		}

		@Override
		public String getLibraryDestination(String libraryName, LibraryScope scope) {
			return "BOOT-INF/lib/";
		}

		@Override
		public String getClassesLocation() {
			return "";
		}

		@Override
		public String getRepackagedClassesLocation() {
			return "BOOT-INF/classes/";
		}

		@Override
		public boolean isExecutable() {
			return true;
		}

	}

	
	public static class Expanded extends Jar {

		@Override
		public String getLauncherClassName() {
			return "org.springframework.boot.loader.PropertiesLauncher";
		}

	}

	
	public static class None extends Jar {

		@Override
		public String getLauncherClassName() {
			return null;
		}

		@Override
		public boolean isExecutable() {
			return false;
		}

	}

	
	public static class War implements Layout {

		private static final Map<LibraryScope, String> SCOPE_DESTINATIONS;

		static {
			Map<LibraryScope, String> map = new HashMap<>();
			map.put(LibraryScope.COMPILE, "WEB-INF/lib/");
			map.put(LibraryScope.CUSTOM, "WEB-INF/lib/");
			map.put(LibraryScope.RUNTIME, "WEB-INF/lib/");
			map.put(LibraryScope.PROVIDED, "WEB-INF/lib-provided/");
			SCOPE_DESTINATIONS = Collections.unmodifiableMap(map);
		}

		@Override
		public String getLauncherClassName() {
			return "org.springframework.boot.loader.WarLauncher";
		}

		@Override
		public String getLibraryDestination(String libraryName, LibraryScope scope) {
			return SCOPE_DESTINATIONS.get(scope);
		}

		@Override
		public String getClassesLocation() {
			return "WEB-INF/classes/";
		}

		@Override
		public boolean isExecutable() {
			return true;
		}

	}

}
