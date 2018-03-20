

package org.springframework.boot.info;

import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.time.Instant;
import java.util.Properties;


public class GitProperties extends InfoProperties {

	public GitProperties(Properties entries) {
		super(processEntries(entries));
	}

	
	public String getBranch() {
		return get("branch");
	}

	
	public String getCommitId() {
		return get("commit.id");
	}

	
	public String getShortCommitId() {
		String shortId = get("commit.id.abbrev");
		if (shortId != null) {
			return shortId;
		}
		String id = getCommitId();
		if (id == null) {
			return null;
		}
		return (id.length() > 7 ? id.substring(0, 7) : id);
	}

	
	public Instant getCommitTime() {
		return getInstant("commit.time");
	}

	private static Properties processEntries(Properties properties) {
		coercePropertyToEpoch(properties, "commit.time");
		coercePropertyToEpoch(properties, "build.time");
		Object commitId = properties.get("commit.id");
		if (commitId != null) {
						properties.put("commit.id.full", commitId);
		}
		return properties;
	}

	private static void coercePropertyToEpoch(Properties properties, String key) {
		String value = properties.getProperty(key);
		if (value != null) {
			properties.setProperty(key, coerceToEpoch(value));
		}
	}

	
	private static String coerceToEpoch(String s) {
		Long epoch = parseEpochSecond(s);
		if (epoch != null) {
			return String.valueOf(epoch);
		}
		SimpleDateFormat format = new SimpleDateFormat("yyyy-MM-dd'T'HH:mm:ssZ");
		try {
			return String.valueOf(format.parse(s).getTime());
		}
		catch (ParseException ex) {
			return s;
		}
	}

	private static Long parseEpochSecond(String s) {
		try {
			return Long.parseLong(s) * 1000;
		}
		catch (NumberFormatException ex) {
			return null;
		}
	}

}
