

package org.springframework.boot.actuate.info;

import java.time.Instant;
import java.util.Map;
import java.util.Properties;

import org.junit.Test;

import org.springframework.boot.actuate.info.InfoPropertiesInfoContributor.Mode;
import org.springframework.boot.info.GitProperties;

import static org.assertj.core.api.Assertions.assertThat;


public class GitInfoContributorTests {

	@Test
	@SuppressWarnings("unchecked")
	public void coerceDate() {
		Properties properties = new Properties();
		properties.put("branch", "master");
		properties.put("commit.time", "2016-03-04T14:36:33+0100");
		GitInfoContributor contributor = new GitInfoContributor(
				new GitProperties(properties));
		Map<String, Object> content = contributor.generateContent();
		assertThat(content.get("commit")).isInstanceOf(Map.class);
		Map<String, Object> commit = (Map<String, Object>) content.get("commit");
		Object commitTime = commit.get("time");
		assertThat(commitTime).isInstanceOf(Instant.class);
		assertThat(((Instant) commitTime).toEpochMilli()).isEqualTo(1457098593000L);
	}

	@Test
	@SuppressWarnings("unchecked")
	public void shortenCommitId() {
		Properties properties = new Properties();
		properties.put("branch", "master");
		properties.put("commit.id", "8e29a0b0d423d2665c6ee5171947c101a5c15681");
		GitInfoContributor contributor = new GitInfoContributor(
				new GitProperties(properties));
		Map<String, Object> content = contributor.generateContent();
		assertThat(content.get("commit")).isInstanceOf(Map.class);
		Map<String, Object> commit = (Map<String, Object>) content.get("commit");
		assertThat(commit.get("id")).isEqualTo("8e29a0b");
	}

	@Test
	@SuppressWarnings("unchecked")
	public void withGitIdAndAbbrev() {
				Properties properties = new Properties();
		properties.put("branch", "master");
		properties.put("commit.id", "1b3cec34f7ca0a021244452f2cae07a80497a7c7");
		properties.put("commit.id.abbrev", "1b3cec3");
		GitInfoContributor contributor = new GitInfoContributor(
				new GitProperties(properties), Mode.FULL);
		Map<String, Object> content = contributor.generateContent();
		Map<String, Object> commit = (Map<String, Object>) content.get("commit");
		assertThat(commit.get("id")).isInstanceOf(Map.class);
		Map<String, Object> id = (Map<String, Object>) commit.get("id");
		assertThat(id.get("full")).isEqualTo("1b3cec34f7ca0a021244452f2cae07a80497a7c7");
		assertThat(id.get("abbrev")).isEqualTo("1b3cec3");
	}

}
