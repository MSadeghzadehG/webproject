

package com.badlogic.gdx.maps;

import java.util.Iterator;

import com.badlogic.gdx.utils.Array;
import com.badlogic.gdx.utils.reflect.ClassReflection;


public class MapObjects implements Iterable<MapObject> {

	private Array<MapObject> objects;

	
	public MapObjects () {
		objects = new Array<MapObject>();
	}

	
	public MapObject get (int index) {
		return objects.get(index);
	}

	
	public MapObject get (String name) {
		for (int i = 0, n = objects.size; i < n; i++) {
			MapObject object = objects.get(i);
			if (name.equals(object.getName())) {
				return object;
			}
		}
		return null;
	}

	
	public int getIndex (String name) {
		return getIndex(get(name));
	}

	
	public int getIndex (MapObject object) {
		return objects.indexOf(object, true);
	}

	
	public int getCount () {
		return objects.size;
	}

	
	public void add (MapObject object) {
		this.objects.add(object);
	}

	
	public void remove (int index) {
		objects.removeIndex(index);
	}

	
	public void remove (MapObject object) {
		objects.removeValue(object, true);
	}

	
	public <T extends MapObject> Array<T> getByType (Class<T> type) {
		return getByType(type, new Array<T>());
	}

	
	public <T extends MapObject> Array<T> getByType (Class<T> type, Array<T> fill) {
		fill.clear();
		for (int i = 0, n = objects.size; i < n; i++) {
			MapObject object = objects.get(i);
			if (ClassReflection.isInstance(type, object)) {
				fill.add((T)object);
			}
		}
		return fill;
	}

	
	@Override
	public Iterator<MapObject> iterator () {
		return objects.iterator();
	}

}
