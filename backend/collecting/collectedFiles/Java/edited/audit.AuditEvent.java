

package org.springframework.boot.actuate.audit;

import java.io.Serializable;
import java.time.Instant;
import java.util.Collections;
import java.util.HashMap;
import java.util.Map;

import com.fasterxml.jackson.annotation.JsonInclude;
import com.fasterxml.jackson.annotation.JsonInclude.Include;

import org.springframework.context.ApplicationEventPublisher;
import org.springframework.context.ApplicationEventPublisherAware;
import org.springframework.util.Assert;


@JsonInclude(Include.NON_EMPTY)
public class AuditEvent implements Serializable {

	private final Instant timestamp;

	private final String principal;

	private final String type;

	private final Map<String, Object> data;

	
	public AuditEvent(String principal, String type, Map<String, Object> data) {
		this(Instant.now(), principal, type, data);
	}

	
	public AuditEvent(String principal, String type, String... data) {
		this(Instant.now(), principal, type, convert(data));
	}

	
	public AuditEvent(Instant timestamp, String principal, String type,
			Map<String, Object> data) {
		Assert.notNull(timestamp, "Timestamp must not be null");
		Assert.notNull(type, "Type must not be null");
		this.timestamp = timestamp;
		this.principal = (principal != null ? principal : "");
		this.type = type;
		this.data = Collections.unmodifiableMap(data);
	}

	private static Map<String, Object> convert(String[] data) {
		Map<String, Object> result = new HashMap<>();
		for (String entry : data) {
			int index = entry.indexOf('=');
			if (index != -1) {
				result.put(entry.substring(0, index), entry.substring(index + 1));
			}
			else {
				result.put(entry, null);
			}
		}
		return result;
	}

	
	public Instant getTimestamp() {
		return this.timestamp;
	}

	
	public String getPrincipal() {
		return this.principal;
	}

	
	public String getType() {
		return this.type;
	}

	
	public Map<String, Object> getData() {
		return this.data;
	}

	@Override
	public String toString() {
		return "AuditEvent [timestamp=" + this.timestamp + ", principal=" + this.principal
				+ ", type=" + this.type + ", data=" + this.data + "]";
	}

}
