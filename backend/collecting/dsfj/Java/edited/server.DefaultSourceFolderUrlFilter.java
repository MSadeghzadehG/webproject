

package org.springframework.boot.devtools.restart.server;

import java.net.URL;
import java.util.Arrays;
import java.util.HashSet;
import java.util.Set;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import org.springframework.util.StringUtils;


public class DefaultSourceFolderUrlFilter implements SourceFolderUrlFilter {

	private static final String[] COMMON_ENDINGS = { "/target/classes", "/bin" };

	private static final Pattern URL_MODULE_PATTERN = Pattern.compile(".*\\/(.+)\\.jar");

	private static final Pattern VERSION_PATTERN = Pattern
			.compile("^-\\d+(?:\\.\\d+)*(?:[.-].+)?$");

	private static final Set<String> SKIPPED_PROJECTS = new HashSet<>(Arrays.asList(
			"spring-boot", "spring-boot-devtools", "spring-boot-autoconfigure",
			"spring-boot-actuator", "spring-boot-starter"));

	@Override
	public boolean isMatch(String sourceFolder, URL url) {
		String jarName = getJarName(url);
		if (!StringUtils.hasLength(jarName)) {
			return false;
		}
		return isMatch(sourceFolder, jarName);
	}

	private String getJarName(URL url) {
		Matcher matcher = URL_MODULE_PATTERN.matcher(url.toString());
		if (matcher.find()) {
			return matcher.group(1);
		}
		return null;
	}

	private boolean isMatch(String sourceFolder, String jarName) {
		sourceFolder = stripTrailingSlash(sourceFolder);
		sourceFolder = stripCommonEnds(sourceFolder);
		String[] folders = StringUtils.delimitedListToStringArray(sourceFolder, "/");
		for (int i = folders.length - 1; i >= 0; i--) {
			if (isFolderMatch(folders[i], jarName)) {
				return true;
			}
		}
		return false;
	}

	private boolean isFolderMatch(String folder, String jarName) {
		if (!jarName.startsWith(folder) || SKIPPED_PROJECTS.contains(folder)) {
			return false;
		}
		String version = jarName.substring(folder.length());
		return version.isEmpty() || VERSION_PATTERN.matcher(version).matches();
	}

	private String stripTrailingSlash(String string) {
		if (string.endsWith("/")) {
			return string.substring(0, string.length() - 1);
		}
		return string;
	}

	private String stripCommonEnds(String string) {
		for (String ending : COMMON_ENDINGS) {
			if (string.endsWith(ending)) {
				return string.substring(0, string.length() - ending.length());
			}
		}
		return string;
	}

}
