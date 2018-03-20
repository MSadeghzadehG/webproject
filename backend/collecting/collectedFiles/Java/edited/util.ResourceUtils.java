

package org.springframework.boot.cli.util;

import java.io.IOException;
import java.net.MalformedURLException;
import java.net.URL;
import java.util.ArrayList;
import java.util.LinkedHashSet;
import java.util.List;
import java.util.Set;

import org.springframework.core.io.ClassPathResource;
import org.springframework.core.io.DefaultResourceLoader;
import org.springframework.core.io.FileSystemResourceLoader;
import org.springframework.core.io.Resource;
import org.springframework.core.io.UrlResource;
import org.springframework.core.io.support.PathMatchingResourcePatternResolver;
import org.springframework.util.Assert;
import org.springframework.util.ClassUtils;
import org.springframework.util.StringUtils;


public abstract class ResourceUtils {

	
	public static final String CLASSPATH_URL_PREFIX = "classpath:";

	
	public static final String ALL_CLASSPATH_URL_PREFIX = "classpath*:";

	
	public static final String FILE_URL_PREFIX = "file:";

	
	public static List<String> getUrls(String path, ClassLoader classLoader) {
		if (classLoader == null) {
			classLoader = ClassUtils.getDefaultClassLoader();
		}
		path = StringUtils.cleanPath(path);
		try {
			return getUrlsFromWildcardPath(path, classLoader);
		}
		catch (Exception ex) {
			throw new IllegalArgumentException(
					"Cannot create URL from path [" + path + "]", ex);
		}
	}

	private static List<String> getUrlsFromWildcardPath(String path,
			ClassLoader classLoader) throws IOException {
		if (path.contains(":")) {
			return getUrlsFromPrefixedWildcardPath(path, classLoader);
		}
		Set<String> result = new LinkedHashSet<>();
		try {
			result.addAll(getUrls(FILE_URL_PREFIX + path, classLoader));
		}
		catch (IllegalArgumentException ex) {
					}
		path = stripLeadingSlashes(path);
		result.addAll(getUrls(ALL_CLASSPATH_URL_PREFIX + path, classLoader));
		return new ArrayList<>(result);
	}

	private static List<String> getUrlsFromPrefixedWildcardPath(String path,
			ClassLoader classLoader) throws IOException {
		Resource[] resources = new PathMatchingResourcePatternResolver(
				new FileSearchResourceLoader(classLoader)).getResources(path);
		List<String> result = new ArrayList<>();
		for (Resource resource : resources) {
			if (resource.exists()) {
				if (resource.getURI().getScheme().equals("file")
						&& resource.getFile().isDirectory()) {
					result.addAll(getChildFiles(resource));
					continue;
				}
				result.add(absolutePath(resource));
			}
		}
		return result;
	}

	private static List<String> getChildFiles(Resource resource) throws IOException {
		Resource[] children = new PathMatchingResourcePatternResolver()
				.getResources(resource.getURL() + "