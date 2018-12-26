

package org.springframework.boot.actuate.session;

import java.time.Instant;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.stream.Collectors;

import org.springframework.boot.actuate.endpoint.annotation.DeleteOperation;
import org.springframework.boot.actuate.endpoint.annotation.Endpoint;
import org.springframework.boot.actuate.endpoint.annotation.ReadOperation;
import org.springframework.boot.actuate.endpoint.annotation.Selector;
import org.springframework.session.FindByIndexNameSessionRepository;
import org.springframework.session.Session;


@Endpoint(id = "sessions")
public class SessionsEndpoint {

	private final FindByIndexNameSessionRepository<? extends Session> sessionRepository;

	
	public SessionsEndpoint(
			FindByIndexNameSessionRepository<? extends Session> sessionRepository) {
		this.sessionRepository = sessionRepository;
	}

	@ReadOperation
	public SessionsReport sessionsForUsername(String username) {
		Map<String, ? extends Session> sessions = this.sessionRepository
				.findByIndexNameAndIndexValue(
						FindByIndexNameSessionRepository.PRINCIPAL_NAME_INDEX_NAME,
						username);
		return new SessionsReport(sessions);
	}

	@ReadOperation
	public SessionDescriptor getSession(@Selector String sessionId) {
		Session session = this.sessionRepository.findById(sessionId);
		if (session == null) {
			return null;
		}
		return new SessionDescriptor(session);
	}

	@DeleteOperation
	public void deleteSession(@Selector String sessionId) {
		this.sessionRepository.deleteById(sessionId);
	}

	
	public static final class SessionsReport {

		private final List<SessionDescriptor> sessions;

		public SessionsReport(Map<String, ? extends Session> sessions) {
			this.sessions = sessions.entrySet().stream()
					.map((s) -> new SessionDescriptor(s.getValue()))
					.collect(Collectors.toList());
		}

		public List<SessionDescriptor> getSessions() {
			return this.sessions;
		}

	}

	
	public static final class SessionDescriptor {

		private final String id;

		private final Set<String> attributeNames;

		private final Instant creationTime;

		private final Instant lastAccessedTime;

		private final long maxInactiveInterval;

		private final boolean expired;

		public SessionDescriptor(Session session) {
			this.id = session.getId();
			this.attributeNames = session.getAttributeNames();
			this.creationTime = session.getCreationTime();
			this.lastAccessedTime = session.getLastAccessedTime();
			this.maxInactiveInterval = session.getMaxInactiveInterval().getSeconds();
			this.expired = session.isExpired();
		}

		public String getId() {
			return this.id;
		}

		public Set<String> getAttributeNames() {
			return this.attributeNames;
		}

		public Instant getCreationTime() {
			return this.creationTime;
		}

		public Instant getLastAccessedTime() {
			return this.lastAccessedTime;
		}

		public long getMaxInactiveInterval() {
			return this.maxInactiveInterval;
		}

		public boolean isExpired() {
			return this.expired;
		}

	}

}
