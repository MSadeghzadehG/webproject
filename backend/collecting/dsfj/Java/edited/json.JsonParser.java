

package org.springframework.boot.json;

import java.util.List;
import java.util.Map;


public interface JsonParser {

	
	Map<String, Object> parseMap(String json) throws JsonParseException;

	
	List<Object> parseList(String json) throws JsonParseException;

}
