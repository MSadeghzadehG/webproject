

package org.springframework.boot.json;

import java.util.List;
import java.util.Map;

import com.google.gson.Gson;
import com.google.gson.GsonBuilder;
import com.google.gson.reflect.TypeToken;


public class GsonJsonParser extends AbstractJsonParser {

	private static final TypeToken<?> MAP_TYPE = new MapTypeToken();

	private static final TypeToken<?> LIST_TYPE = new ListTypeToken();

	private Gson gson = new GsonBuilder().create();

	@Override
	public Map<String, Object> parseMap(String json) {
		return parseMap(json,
				(trimmed) -> this.gson.fromJson(trimmed, MAP_TYPE.getType()));
	}

	@Override
	public List<Object> parseList(String json) {
		return parseList(json,
				(trimmed) -> this.gson.fromJson(trimmed, LIST_TYPE.getType()));
	}

	private static final class MapTypeToken extends TypeToken<Map<String, Object>> {

	}

	private static final class ListTypeToken extends TypeToken<List<Object>> {

	}

}
