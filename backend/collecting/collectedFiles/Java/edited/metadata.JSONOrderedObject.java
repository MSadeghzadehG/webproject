

package org.springframework.boot.configurationprocessor.metadata;

import java.util.Iterator;
import java.util.LinkedHashSet;
import java.util.Set;

import org.springframework.boot.configurationprocessor.json.JSONException;
import org.springframework.boot.configurationprocessor.json.JSONObject;


@SuppressWarnings("rawtypes")
class JSONOrderedObject extends JSONObject {

	private Set<String> keys = new LinkedHashSet<>();

	@Override
	public JSONObject put(String key, Object value) throws JSONException {
		this.keys.add(key);
		return super.put(key, value);
	}

	@Override
	public Iterator keys() {
		return this.keys.iterator();
	}

}
