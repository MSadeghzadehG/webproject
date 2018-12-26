

package org.springframework.boot.actuate.info;

import java.util.Collections;
import java.util.LinkedHashMap;
import java.util.Map;

import com.fasterxml.jackson.annotation.JsonAnyGetter;
import com.fasterxml.jackson.annotation.JsonInclude;
import com.fasterxml.jackson.annotation.JsonInclude.Include;


@JsonInclude(Include.NON_EMPTY)
public final class Info {

	private final Map<String, Object> details;

	private Info(Builder builder) {
		Map<String, Object> content = new LinkedHashMap<>();
		content.putAll(builder.content);
		this.details = Collections.unmodifiableMap(content);
	}

	
	@JsonAnyGetter
	public Map<String, Object> getDetails() {
		return this.details;
	}

	public Object get(String id) {
		return this.details.get(id);
	}

	@SuppressWarnings("unchecked")
	public <T> T get(String id, Class<T> type) {
		Object value = get(id);
		if (value != null && type != null && !type.isInstance(value)) {
			throw new IllegalStateException("Info entry is not of required type ["
					+ type.getName() + "]: " + value);
		}
		return (T) value;
	}

	@Override
	public boolean equals(Object obj) {
		if (obj == this) {
			return true;
		}
		if (obj != null && obj instanceof Info) {
			Info other = (Info) obj;
			return this.details.equals(other.details);
		}
		return false;
	}

	@Override
	public int hashCode() {
		return this.details.hashCode();
	}

	@Override
	public String toString() {
		return getDetails().toString();
	}

	
	public static class Builder {

		private final Map<String, Object> content;

		public Builder() {
			this.content = new LinkedHashMap<>();
		}

		
		public Builder withDetail(String key, Object value) {
			this.content.put(key, value);
			return this;
		}

		
		public Builder withDetails(Map<String, Object> details) {
			this.content.putAll(details);
			return this;
		}

		
		public Info build() {
			return new Info(this);
		}

	}

}
