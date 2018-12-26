

package org.springframework.boot.actuate.audit;

import java.time.Instant;
import java.util.LinkedList;
import java.util.List;

import org.springframework.util.Assert;


public class InMemoryAuditEventRepository implements AuditEventRepository {

	private static final int DEFAULT_CAPACITY = 1000;

	private final Object monitor = new Object();

	
	private AuditEvent[] events;

	private volatile int tail = -1;

	public InMemoryAuditEventRepository() {
		this(DEFAULT_CAPACITY);
	}

	public InMemoryAuditEventRepository(int capacity) {
		this.events = new AuditEvent[capacity];
	}

	
	public void setCapacity(int capacity) {
		synchronized (this.monitor) {
			this.events = new AuditEvent[capacity];
		}
	}

	@Override
	public void add(AuditEvent event) {
		Assert.notNull(event, "AuditEvent must not be null");
		synchronized (this.monitor) {
			this.tail = (this.tail + 1) % this.events.length;
			this.events[this.tail] = event;
		}
	}

	@Override
	public List<AuditEvent> find(String principal, Instant after, String type) {
		LinkedList<AuditEvent> events = new LinkedList<>();
		synchronized (this.monitor) {
			for (int i = 0; i < this.events.length; i++) {
				AuditEvent event = resolveTailEvent(i);
				if (event != null && isMatch(principal, after, type, event)) {
					events.addFirst(event);
				}
			}
		}
		return events;
	}

	private boolean isMatch(String principal, Instant after, String type,
			AuditEvent event) {
		boolean match = true;
		match = match && (principal == null || event.getPrincipal().equals(principal));
		match = match && (after == null || event.getTimestamp().isAfter(after));
		match = match && (type == null || event.getType().equals(type));
		return match;
	}

	private AuditEvent resolveTailEvent(int offset) {
		int index = ((this.tail + this.events.length - offset) % this.events.length);
		return this.events[index];
	}

}
