

package org.springframework.boot.actuate.audit;

import java.time.OffsetDateTime;
import java.util.List;

import org.springframework.boot.actuate.endpoint.annotation.Endpoint;
import org.springframework.boot.actuate.endpoint.annotation.ReadOperation;
import org.springframework.lang.Nullable;
import org.springframework.util.Assert;


@Endpoint(id = "auditevents")
public class AuditEventsEndpoint {

	private final AuditEventRepository auditEventRepository;

	public AuditEventsEndpoint(AuditEventRepository auditEventRepository) {
		Assert.notNull(auditEventRepository, "AuditEventRepository must not be null");
		this.auditEventRepository = auditEventRepository;
	}

	@ReadOperation
	public AuditEventsDescriptor events(@Nullable String principal,
			@Nullable OffsetDateTime after, @Nullable String type) {
		return new AuditEventsDescriptor(this.auditEventRepository.find(principal,
				after == null ? null : after.toInstant(), type));
	}

	
	public static final class AuditEventsDescriptor {

		private final List<AuditEvent> events;

		private AuditEventsDescriptor(List<AuditEvent> events) {
			this.events = events;
		}

		public List<AuditEvent> getEvents() {
			return this.events;
		}

	}

}
