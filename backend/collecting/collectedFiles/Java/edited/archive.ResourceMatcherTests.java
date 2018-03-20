

package org.springframework.boot.cli.command.archive;

import java.io.File;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.List;

import org.assertj.core.api.Condition;
import org.junit.Test;

import org.springframework.boot.cli.command.archive.ResourceMatcher.MatchedResource;
import org.springframework.test.util.ReflectionTestUtils;

import static org.assertj.core.api.Assertions.assertThat;


public class ResourceMatcherTests {

	@Test
	public void nonExistentRoot() throws IOException {
		ResourceMatcher resourceMatcher = new ResourceMatcher(
				Arrays.asList("alphaexcluded"));
		List<MatchedResource> matchedResources = resourceMatcher
				.find(Arrays.asList(new File("does-not-exist")));
		assertThat(matchedResources).isEmpty();
	}

	@SuppressWarnings("unchecked")
	@Test
	public void defaults() {
		ResourceMatcher resourceMatcher = new ResourceMatcher(Arrays.asList(""),
				Arrays.asList(""));
		Collection<String> includes = (Collection<String>) ReflectionTestUtils
				.getField(resourceMatcher, "includes");
		Collection<String> excludes = (Collection<String>) ReflectionTestUtils
				.getField(resourceMatcher, "excludes");
		assertThat(includes).contains("static*.jar");
	}

	@Test
	public void excludedWins() throws Exception {
		ResourceMatcher resourceMatcher = new ResourceMatcher(Arrays.asList("*"),
				Arrays.asList("***.jar"));
		List<MatchedResource> found = resourceMatcher
				.find(Arrays.asList(new File("src/test/resources")));
		assertThat(found).areNot(new Condition<MatchedResource>() {

			@Override
			public boolean matches(MatchedResource value) {
				return value.getFile().getName().equals("foo.jar");
			}

		});
	}

	@SuppressWarnings("unchecked")
	@Test
	public void includedDeltas() {
		ResourceMatcher resourceMatcher = new ResourceMatcher(Arrays.asList("-static*.jar"));
		Collection<String> includes = (Collection<String>) ReflectionTestUtils
				.getField(resourceMatcher, "includes");
		Collection<String> excludes = (Collection<String>) ReflectionTestUtils
				.getField(resourceMatcher, "excludes");
		assertThat(includes).contains("foo.jar");
		assertThat(includes).contains("templates*.jar");
	}

	@SuppressWarnings("unchecked")
	@Test
	public void excludedDeltas() {
		ResourceMatcher resourceMatcher = new ResourceMatcher(Arrays.asList(""),
				Arrays.asList("-***.jar"));
		Collection<String> excludes = (Collection<String>) ReflectionTestUtils
				.getField(resourceMatcher, "excludes");
		assertThat(excludes).doesNotContain("***.jar");
	}

	@Test
	public void jarFileAlwaysMatches() throws Exception {
		ResourceMatcher resourceMatcher = new ResourceMatcher(Arrays.asList("*"),
				Arrays.asList("***.jar"));
		List<MatchedResource> found = resourceMatcher
				.find(Arrays.asList(new File("src/test/resources/templates"),
						new File("src/test/resources/foo.jar")));
		assertThat(found).areAtLeastOne(new Condition<MatchedResource>() {

			@Override
			public boolean matches(MatchedResource value) {
				return value.getFile().getName().equals("foo.jar") && value.isRoot();
			}

		});
	}

	@Test
	public void resourceMatching() throws IOException {
		ResourceMatcher resourceMatcher = new ResourceMatcher(
				Arrays.asList("alphaexcluded"));
		List<MatchedResource> matchedResources = resourceMatcher
				.find(Arrays.asList(new File("src/test/resources/resource-matcher/one"),
						new File("src/test/resources/resource-matcher/two"),
						new File("src/test/resources/resource-matcher/three")));
		List<String> paths = new ArrayList<>();
		for (MatchedResource resource : matchedResources) {
			paths.add(resource.getName());
		}
		assertThat(paths).containsOnly("alpha/nested/fileA", "bravo/fileC", "fileD",
				"bravo/fileE", "fileF", "three");
	}

}
