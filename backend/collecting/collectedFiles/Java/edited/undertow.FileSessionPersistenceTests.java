

package org.springframework.boot.web.embedded.undertow;

import java.io.File;
import java.io.IOException;
import java.util.Date;
import java.util.LinkedHashMap;
import java.util.Map;

import io.undertow.servlet.api.SessionPersistenceManager.PersistentSession;
import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.TemporaryFolder;

import static org.assertj.core.api.Assertions.assertThat;


public class FileSessionPersistenceTests {

	@Rule
	public TemporaryFolder temp = new TemporaryFolder();

	private File dir;

	private FileSessionPersistence persistence;

	private ClassLoader classLoader = Thread.currentThread().getContextClassLoader();

	private Date expiration = new Date(System.currentTimeMillis() + 10000);

	@Before
	public void setup() throws IOException {
		this.dir = this.temp.newFolder();
		this.persistence = new FileSessionPersistence(this.dir);
	}

	@Test
	public void loadsNullForMissingFile() {
		Map<String, PersistentSession> attributes = this.persistence
				.loadSessionAttributes("test", this.classLoader);
		assertThat(attributes).isNull();
	}

	@Test
	public void persistAndLoad() {
		Map<String, PersistentSession> sessionData = new LinkedHashMap<>();
		Map<String, Object> data = new LinkedHashMap<>();
		data.put("spring", "boot");
		PersistentSession session = new PersistentSession(this.expiration, data);
		sessionData.put("abc", session);
		this.persistence.persistSessions("test", sessionData);
		Map<String, PersistentSession> restored = this.persistence
				.loadSessionAttributes("test", this.classLoader);
		assertThat(restored).isNotNull();
		assertThat(restored.get("abc").getExpiration()).isEqualTo(this.expiration);
		assertThat(restored.get("abc").getSessionData().get("spring")).isEqualTo("boot");
	}

	@Test
	public void dontRestoreExpired() {
		Date expired = new Date(System.currentTimeMillis() - 1000);
		Map<String, PersistentSession> sessionData = new LinkedHashMap<>();
		Map<String, Object> data = new LinkedHashMap<>();
		data.put("spring", "boot");
		PersistentSession session = new PersistentSession(expired, data);
		sessionData.put("abc", session);
		this.persistence.persistSessions("test", sessionData);
		Map<String, PersistentSession> restored = this.persistence
				.loadSessionAttributes("test", this.classLoader);
		assertThat(restored).isNotNull();
		assertThat(restored.containsKey("abc")).isFalse();
	}

	@Test
	public void deleteFileOnClear() {
		File sessionFile = new File(this.dir, "test.session");
		Map<String, PersistentSession> sessionData = new LinkedHashMap<>();
		this.persistence.persistSessions("test", sessionData);
		assertThat(sessionFile.exists()).isTrue();
		this.persistence.clear("test");
		assertThat(sessionFile.exists()).isFalse();
	}

}
