

package org.springframework.boot.json;

import java.util.List;
import java.util.Map;

import org.yaml.snakeyaml.Yaml;


public class YamlJsonParser extends AbstractJsonParser {

	@Override
	@SuppressWarnings("unchecked")
	public Map<String, Object> parseMap(String json) {
		return parseMap(json, (trimmed) -> new Yaml().loadAs(trimmed, Map.class));
	}

	@Override
	@SuppressWarnings("unchecked")
	public List<Object> parseList(String json) {
		return parseList(json, (trimmed) -> new Yaml().loadAs(trimmed, List.class));
	}

}
