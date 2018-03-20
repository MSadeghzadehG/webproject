

package org.springframework.boot.web.embedded.undertow;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.io.Serializable;
import java.util.Date;
import java.util.LinkedHashMap;
import java.util.Map;

import io.undertow.servlet.UndertowServletLogger;
import io.undertow.servlet.api.SessionPersistenceManager;

import org.springframework.core.ConfigurableObjectInputStream;


class FileSessionPersistence implements SessionPersistenceManager {

	private final File dir;

	FileSessionPersistence(File dir) {
		this.dir = dir;
	}

	@Override
	public void persistSessions(String deploymentName,
			Map<String, PersistentSession> sessionData) {
		try {
			save(sessionData, getSessionFile(deploymentName));
		}
		catch (Exception ex) {
			UndertowServletLogger.ROOT_LOGGER.failedToPersistSessions(ex);
		}
	}

	private void save(Map<String, PersistentSession> sessionData, File file)
			throws IOException {
		try (ObjectOutputStream stream = new ObjectOutputStream(
				new FileOutputStream(file))) {
			save(sessionData, stream);
		}
	}

	private void save(Map<String, PersistentSession> sessionData,
			ObjectOutputStream stream) throws IOException {
		Map<String, Serializable> session = new LinkedHashMap<>();
		for (Map.Entry<String, PersistentSession> entry : sessionData.entrySet()) {
			session.put(entry.getKey(),
					new SerializablePersistentSession(entry.getValue()));
		}
		stream.writeObject(session);
	}

	@Override
	public Map<String, PersistentSession> loadSessionAttributes(String deploymentName,
			final ClassLoader classLoader) {
		try {
			File file = getSessionFile(deploymentName);
			if (file.exists()) {
				return load(file, classLoader);
			}
		}
		catch (Exception ex) {
			UndertowServletLogger.ROOT_LOGGER.failedtoLoadPersistentSessions(ex);
		}
		return null;
	}

	private Map<String, PersistentSession> load(File file, ClassLoader classLoader)
			throws IOException, ClassNotFoundException {
		try (ObjectInputStream stream = new ConfigurableObjectInputStream(
				new FileInputStream(file), classLoader)) {
			return load(stream);
		}
	}

	private Map<String, PersistentSession> load(ObjectInputStream stream)
			throws ClassNotFoundException, IOException {
		Map<String, SerializablePersistentSession> session = readSession(stream);
		long time = System.currentTimeMillis();
		Map<String, PersistentSession> result = new LinkedHashMap<>();
		for (Map.Entry<String, SerializablePersistentSession> entry : session
				.entrySet()) {
			PersistentSession entrySession = entry.getValue().getPersistentSession();
			if (entrySession.getExpiration().getTime() > time) {
				result.put(entry.getKey(), entrySession);
			}
		}
		return result;
	}

	@SuppressWarnings("unchecked")
	private Map<String, SerializablePersistentSession> readSession(
			ObjectInputStream stream) throws ClassNotFoundException, IOException {
		return ((Map<String, SerializablePersistentSession>) stream.readObject());
	}

	private File getSessionFile(String deploymentName) {
		if (!this.dir.exists()) {
			this.dir.mkdirs();
		}
		return new File(this.dir, deploymentName + ".session");
	}

	@Override
	public void clear(String deploymentName) {
		getSessionFile(deploymentName).delete();
	}

	
	static class SerializablePersistentSession implements Serializable {

		private static final long serialVersionUID = 0L;

		private final Date expiration;

		private final Map<String, Object> sessionData;

		SerializablePersistentSession(PersistentSession session) {
			this.expiration = session.getExpiration();
			this.sessionData = new LinkedHashMap<>(session.getSessionData());
		}

		public PersistentSession getPersistentSession() {
			return new PersistentSession(this.expiration, this.sessionData);
		}

	}

}
