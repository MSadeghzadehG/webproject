

package org.springframework.boot.json;

import java.util.List;
import java.util.Map;

import com.fasterxml.jackson.core.type.TypeReference;
import com.fasterxml.jackson.databind.ObjectMapper;


public class JacksonJsonParser extends AbstractJsonParser {

	private static final TypeReference<?> MAP_TYPE = new MapTypeReference();

	private static final TypeReference<?> LIST_TYPE = new ListTypeReference();

	private ObjectMapper objectMapper; 
	@Override
	public Map<String, Object> parseMap(String json) {
		return tryParse(() -> getObjectMapper().readValue(json, MAP_TYPE),
				Exception.class);
	}

	@Override
	public List<Object> parseList(String json) {
		return tryParse(() -> getObjectMapper().readValue(json, LIST_TYPE),
				Exception.class);
	}

	private ObjectMapper getObjectMapper() {
		if (this.objectMapper == null) {
			this.objectMapper = new ObjectMapper();
		}
		return this.objectMapper;
	}

	private static class MapTypeReference extends TypeReference<Map<String, Object>> {

	}

	private static class ListTypeReference extends TypeReference<List<Object>> {

	}

}
