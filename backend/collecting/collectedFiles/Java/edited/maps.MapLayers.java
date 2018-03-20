

package com.badlogic.gdx.maps;

import java.util.Iterator;

import com.badlogic.gdx.utils.Array;
import com.badlogic.gdx.utils.reflect.ClassReflection;


public class MapLayers implements Iterable<MapLayer> {
	private Array<MapLayer> layers = new Array<MapLayer>();

	
	public MapLayer get (int index) {
		return layers.get(index);
	}

	
	public MapLayer get (String name) {
		for (int i = 0, n = layers.size; i < n; i++) {
			MapLayer layer = layers.get(i);
			if (name.equals(layer.getName())) {
				return layer;
			}
		}
		return null;
	}

	
	public int getIndex (String name) {
		return getIndex(get(name));
	}

	
	public int getIndex (MapLayer layer) {
		return layers.indexOf(layer, true);
	}

	
	public int getCount () {
		return layers.size;
	}

	
	public void add (MapLayer layer) {
		this.layers.add(layer);
	}

	
	public void remove (int index) {
		layers.removeIndex(index);
	}

	
	public void remove (MapLayer layer) {
		layers.removeValue(layer, true);
	}

	
	public int size () {
		return layers.size;
	}

	
	public <T extends MapLayer> Array<T> getByType (Class<T> type) {
		return getByType(type, new Array<T>());
	}

	
	public <T extends MapLayer> Array<T> getByType (Class<T> type, Array<T> fill) {
		fill.clear();
		for (int i = 0, n = layers.size; i < n; i++) {
			MapLayer layer = layers.get(i);
			if (ClassReflection.isInstance(type, layer)) {
				fill.add((T)layer);
			}
		}
		return fill;
	}

	
	@Override
	public Iterator<MapLayer> iterator () {
		return layers.iterator();
	}

}
