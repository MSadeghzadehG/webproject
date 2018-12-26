

package org.springframework.boot.actuate.audit;

import java.time.OffsetDateTime;
import java.util.Collections;
import java.util.List;

import org.junit.Test;

import static org.assertj.core.api.Assertions.assertThat;
import static org.mockito.BDDMockito.given;
import static org.mockito.Mockito.mock;


public class AuditEventsEndpointTests {

	private final AuditEventRepository repository = mock(AuditEventRepository.class);

	private final AuditEventsEndpoint endpoint = new AuditEventsEndpoint(this.repository);

	private final AuditEvent event = new AuditEvent("principal", "type",
			Collections.singletonMap("a", "alpha"));

	@Test
	public void eventsWithType() {
		given(this.repository.find(null, null, "type"))
				.willReturn(Collections.singletonList(this.event));
		List<AuditEvent> result = this.endpoint.events(null, null, "type").getEvents();
		assertThat(result).isEqualTo(Collections.singletonList(this.event));
	}

	@Test
	public void eventsCreatedAfter() {
		OffsetDateTime now = OffsetDateTime.now();
		given(this.repository.find(null, now.toInstant(), null))
				.willReturn(Collections.singletonList(this.event));
		List<AuditEvent> result = this.endpoint.events(null, now, null).getEvents();
		assertThat(result).isEqualTo(Collections.singletonList(this.event));
	}

	@Test
	public void eventsWithPrincipal() {
		given(this.repository.find("Joan", null, null))
				.willReturn(Collections.singletonList(this.event));
		List<AuditEvent> result = this.endpoint.events("Joan", null, null).getEvents();
		assertThat(result).isEqualTo(Collections.singletonList(this.event));
	}

}
