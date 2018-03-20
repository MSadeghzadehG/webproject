

package com.badlogic.gdx.maps;

import java.util.Iterator;

import com.badlogic.gdx.graphics.Color;
import com.badlogic.gdx.math.Vector2;
import com.badlogic.gdx.utils.ObjectMap;


public class MapProperties {

	private ObjectMap<String, Object> properties;

	
	public MapProperties () {
		properties = new ObjectMap<String, Object>();
	}

	
	public boolean containsKey (String key) {
		return properties.containsKey(key);
	}

	
	public Object get (String key) {
		return properties.get(key);
	}

	
	public <T> T get (String key, Class<T> clazz) {
		return (T)get(key);
	}

	
	public <T> T get (String key, T defaultValue, Class<T> clazz) {
		Object object = get(key);
		return object == null ? defaultValue : (T)object;
	}

	
	public void put (String key, Object value) {
		properties.put(key, value);
	}

	
	public void putAll (MapProperties properties) {
		this.properties.putAll(properties.properties);
	}

	
	public void remove (String key) {
		properties.remove(key);
	}

	
	public void clear () {
		properties.clear();
	}

	
	public Iterator<String> getKeys () {
		return properties.keys();
	}

	
	public Iterator<Object> getValues () {
		return properties.values();
	}

}
