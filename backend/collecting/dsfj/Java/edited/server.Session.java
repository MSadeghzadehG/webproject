

package org.springframework.boot.web.servlet.server;

import java.io.File;
import java.time.Duration;
import java.time.temporal.ChronoUnit;
import java.util.Set;

import org.springframework.boot.convert.DurationUnit;


public class Session {

	
	@DurationUnit(ChronoUnit.SECONDS)
	private Duration timeout = Duration.ofMinutes(30);

	
	private Set<Session.SessionTrackingMode> trackingModes;

	
	private boolean persistent;

	
	private File storeDir;

	private final Cookie cookie = new Cookie();

	private final SessionStoreDirectory sessionStoreDirectory = new SessionStoreDirectory();

	public Cookie getCookie() {
		return this.cookie;
	}

	public Duration getTimeout() {
		return this.timeout;
	}

	public void setTimeout(Duration timeout) {
		this.timeout = timeout;
	}

	public Set<Session.SessionTrackingMode> getTrackingModes() {
		return this.trackingModes;
	}

	public void setTrackingModes(Set<Session.SessionTrackingMode> trackingModes) {
		this.trackingModes = trackingModes;
	}

	public boolean isPersistent() {
		return this.persistent;
	}

	public void setPersistent(boolean persistent) {
		this.persistent = persistent;
	}

	public File getStoreDir() {
		return this.storeDir;
	}

	public void setStoreDir(File storeDir) {
		this.sessionStoreDirectory.setDirectory(storeDir);
		this.storeDir = storeDir;
	}

	SessionStoreDirectory getSessionStoreDirectory() {
		return this.sessionStoreDirectory;
	}

	
	public static class Cookie {

		
		private String name;

		
		private String domain;

		
		private String path;

		
		private String comment;

		
		private Boolean httpOnly;

		
		private Boolean secure;

		
		@DurationUnit(ChronoUnit.SECONDS)
		private Duration maxAge;

		public String getName() {
			return this.name;
		}

		public void setName(String name) {
			this.name = name;
		}

		public String getDomain() {
			return this.domain;
		}

		public void setDomain(String domain) {
			this.domain = domain;
		}

		public String getPath() {
			return this.path;
		}

		public void setPath(String path) {
			this.path = path;
		}

		public String getComment() {
			return this.comment;
		}

		public void setComment(String comment) {
			this.comment = comment;
		}

		public Boolean getHttpOnly() {
			return this.httpOnly;
		}

		public void setHttpOnly(Boolean httpOnly) {
			this.httpOnly = httpOnly;
		}

		public Boolean getSecure() {
			return this.secure;
		}

		public void setSecure(Boolean secure) {
			this.secure = secure;
		}

		public Duration getMaxAge() {
			return this.maxAge;
		}

		public void setMaxAge(Duration maxAge) {
			this.maxAge = maxAge;
		}

	}

	
	public enum SessionTrackingMode {

		
		COOKIE,

		
		URL,

		
		SSL

	}

}
