

package com.badlogic.gdx.utils;

import java.util.Iterator;
import java.util.NoSuchElementException;

import com.badlogic.gdx.math.MathUtils;
import com.badlogic.gdx.utils.ObjectMap.Entry;
import com.badlogic.gdx.utils.reflect.ArrayReflection;


public class ArrayMap<K, V> implements Iterable<ObjectMap.Entry<K, V>> {
	public K[] keys;
	public V[] values;
	public int size;
	public boolean ordered;

	private Entries entries1, entries2;
	private Values valuesIter1, valuesIter2;
	private Keys keysIter1, keysIter2;

	
	public ArrayMap () {
		this(true, 16);
	}

	
	public ArrayMap (int capacity) {
		this(true, capacity);
	}

	
	public ArrayMap (boolean ordered, int capacity) {
		this.ordered = ordered;
		keys = (K[])new Object[capacity];
		values = (V[])new Object[capacity];
	}

	
	public ArrayMap (boolean ordered, int capacity, Class keyArrayType, Class valueArrayType) {
		this.ordered = ordered;
		keys = (K[])ArrayReflection.newInstance(keyArrayType, capacity);
		values = (V[])ArrayReflection.newInstance(valueArrayType, capacity);
	}

	
	public ArrayMap (Class keyArrayType, Class valueArrayType) {
		this(false, 16, keyArrayType, valueArrayType);
	}

	
	public ArrayMap (ArrayMap array) {
		this(array.ordered, array.size, array.keys.getClass().getComponentType(), array.values.getClass().getComponentType());
		size = array.size;
		System.arraycopy(array.keys, 0, keys, 0, size);
		System.arraycopy(array.values, 0, values, 0, size);
	}

	public int put (K key, V value) {
		int index = indexOfKey(key);
		if (index == -1) {
			if (size == keys.length) resize(Math.max(8, (int)(size * 1.75f)));
			index = size++;
		}
		keys[index] = key;
		values[index] = value;
		return index;
	}

	public int put (K key, V value, int index) {
		int existingIndex = indexOfKey(key);
		if (existingIndex != -1)
			removeIndex(existingIndex);
		else if (size == keys.length) 			resize(Math.max(8, (int)(size * 1.75f)));
		System.arraycopy(keys, index, keys, index + 1, size - index);
		System.arraycopy(values, index, values, index + 1, size - index);
		keys[index] = key;
		values[index] = value;
		size++;
		return index;
	}

	public void putAll (ArrayMap map) {
		putAll(map, 0, map.size);
	}

	public void putAll (ArrayMap map, int offset, int length) {
		if (offset + length > map.size)
			throw new IllegalArgumentException("offset + length must be <= size: " + offset + " + " + length + " <= " + map.size);
		int sizeNeeded = size + length - offset;
		if (sizeNeeded >= keys.length) resize(Math.max(8, (int)(sizeNeeded * 1.75f)));
		System.arraycopy(map.keys, offset, keys, size, length);
		System.arraycopy(map.values, offset, values, size, length);
		size += length;
	}

	
	public V get (K key) {
		Object[] keys = this.keys;
		int i = size - 1;
		if (key == null) {
			for (; i >= 0; i--)
				if (keys[i] == key) return values[i];
		} else {
			for (; i >= 0; i--)
				if (key.equals(keys[i])) return values[i];
		}
		return null;
	}

	
	public K getKey (V value, boolean identity) {
		Object[] values = this.values;
		int i = size - 1;
		if (identity || value == null) {
			for (; i >= 0; i--)
				if (values[i] == value) return keys[i];
		} else {
			for (; i >= 0; i--)
				if (value.equals(values[i])) return keys[i];
		}
		return null;
	}

	public K getKeyAt (int index) {
		if (index >= size) throw new IndexOutOfBoundsException(String.valueOf(index));
		return keys[index];
	}

	public V getValueAt (int index) {
		if (index >= size) throw new IndexOutOfBoundsException(String.valueOf(index));
		return values[index];
	}

	public K firstKey () {
		if (size == 0) throw new IllegalStateException("Map is empty.");
		return keys[0];
	}

	public V firstValue () {
		if (size == 0) throw new IllegalStateException("Map is empty.");
		return values[0];
	}

	public void setKey (int index, K key) {
		if (index >= size) throw new IndexOutOfBoundsException(String.valueOf(index));
		keys[index] = key;
	}

	public void setValue (int index, V value) {
		if (index >= size) throw new IndexOutOfBoundsException(String.valueOf(index));
		values[index] = value;
	}

	public void insert (int index, K key, V value) {
		if (index > size) throw new IndexOutOfBoundsException(String.valueOf(index));
		if (size == keys.length) resize(Math.max(8, (int)(size * 1.75f)));
		if (ordered) {
			System.arraycopy(keys, index, keys, index + 1, size - index);
			System.arraycopy(values, index, values, index + 1, size - index);
		} else {
			keys[size] = keys[index];
			values[size] = values[index];
		}
		size++;
		keys[index] = key;
		values[index] = value;
	}

	public boolean containsKey (K key) {
		K[] keys = this.keys;
		int i = size - 1;
		if (key == null) {
			while (i >= 0)
				if (keys[i--] == key) return true;
		} else {
			while (i >= 0)
				if (key.equals(keys[i--])) return true;
		}
		return false;
	}

	
	public boolean containsValue (V value, boolean identity) {
		V[] values = this.values;
		int i = size - 1;
		if (identity || value == null) {
			while (i >= 0)
				if (values[i--] == value) return true;
		} else {
			while (i >= 0)
				if (value.equals(values[i--])) return true;
		}
		return false;
	}

	public int indexOfKey (K key) {
		Object[] keys = this.keys;
		if (key == null) {
			for (int i = 0, n = size; i < n; i++)
				if (keys[i] == key) return i;
		} else {
			for (int i = 0, n = size; i < n; i++)
				if (key.equals(keys[i])) return i;
		}
		return -1;
	}

	public int indexOfValue (V value, boolean identity) {
		Object[] values = this.values;
		if (identity || value == null) {
			for (int i = 0, n = size; i < n; i++)
				if (values[i] == value) return i;
		} else {
			for (int i = 0, n = size; i < n; i++)
				if (value.equals(values[i])) return i;
		}
		return -1;
	}

	public V removeKey (K key) {
		Object[] keys = this.keys;
		if (key == null) {
			for (int i = 0, n = size; i < n; i++) {
				if (keys[i] == key) {
					V value = values[i];
					removeIndex(i);
					return value;
				}
			}
		} else {
			for (int i = 0, n = size; i < n; i++) {
				if (key.equals(keys[i])) {
					V value = values[i];
					removeIndex(i);
					return value;
				}
			}
		}
		return null;
	}

	public boolean removeValue (V value, boolean identity) {
		Object[] values = this.values;
		if (identity || value == null) {
			for (int i = 0, n = size; i < n; i++) {
				if (values[i] == value) {
					removeIndex(i);
					return true;
				}
			}
		} else {
			for (int i = 0, n = size; i < n; i++) {
				if (value.equals(values[i])) {
					removeIndex(i);
					return true;
				}
			}
		}
		return false;
	}

	
	public void removeIndex (int index) {
		if (index >= size) throw new IndexOutOfBoundsException(String.valueOf(index));
		Object[] keys = this.keys;
		size--;
		if (ordered) {
			System.arraycopy(keys, index + 1, keys, index, size - index);
			System.arraycopy(values, index + 1, values, index, size - index);
		} else {
			keys[index] = keys[size];
			values[index] = values[size];
		}
		keys[size] = null;
		values[size] = null;
	}

	
	public K peekKey () {
		return keys[size - 1];
	}

	
	public V peekValue () {
		return values[size - 1];
	}

	
	public void clear (int maximumCapacity) {
		if (keys.length <= maximumCapacity) {
			clear();
			return;
		}
		size = 0;
		resize(maximumCapacity);
	}

	public void clear () {
		K[] keys = this.keys;
		V[] values = this.values;
		for (int i = 0, n = size; i < n; i++) {
			keys[i] = null;
			values[i] = null;
		}
		size = 0;
	}

	
	public void shrink () {
		if (keys.length == size) return;
		resize(size);
	}

	
	public void ensureCapacity (int additionalCapacity) {
		int sizeNeeded = size + additionalCapacity;
		if (sizeNeeded >= keys.length) resize(Math.max(8, sizeNeeded));
	}

	protected void resize (int newSize) {
		K[] newKeys = (K[])ArrayReflection.newInstance(keys.getClass().getComponentType(), newSize);
		System.arraycopy(keys, 0, newKeys, 0, Math.min(size, newKeys.length));
		this.keys = newKeys;

		V[] newValues = (V[])ArrayReflection.newInstance(values.getClass().getComponentType(), newSize);
		System.arraycopy(values, 0, newValues, 0, Math.min(size, newValues.length));
		this.values = newValues;
	}

	public void reverse () {
		for (int i = 0, lastIndex = size - 1, n = size / 2; i < n; i++) {
			int ii = lastIndex - i;
			K tempKey = keys[i];
			keys[i] = keys[ii];
			keys[ii] = tempKey;

			V tempValue = values[i];
			values[i] = values[ii];
			values[ii] = tempValue;
		}
	}

	public void shuffle () {
		for (int i = size - 1; i >= 0; i--) {
			int ii = MathUtils.random(i);
			K tempKey = keys[i];
			keys[i] = keys[ii];
			keys[ii] = tempKey;

			V tempValue = values[i];
			values[i] = values[ii];
			values[ii] = tempValue;
		}
	}

	
	public void truncate (int newSize) {
		if (size <= newSize) return;
		for (int i = newSize; i < size; i++) {
			keys[i] = null;
			values[i] = null;
		}
		size = newSize;
	}

	public int hashCode () {
		K[] keys = this.keys;
		V[] values = this.values;
		int h = 0;
		for (int i = 0, n = size; i < n; i++) {
			K key = keys[i];
			V value = values[i];
			if (key != null) h += key.hashCode() * 31;
			if (value != null) h += value.hashCode();
		}
		return h;
	}

	public boolean equals (Object obj) {
		if (obj == this) return true;
		if (!(obj instanceof ArrayMap)) return false;
		ArrayMap<K, V> other = (ArrayMap) obj;
		if (other.size != size) return false;
		K[] keys = this.keys;
		V[] values = this.values;
		for (int i = 0, n = size; i < n; i++) {
			K key = keys[i];
			V value = values[i];
			if (value == null) {
				if (!other.containsKey(key) || other.get(key) != null) {
					return false;
				}
			} else {
				if (!value.equals(other.get(key))) {
					return false;
				}
			}
		}
		return true;
	}

	public String toString () {
		if (size == 0) return "{}";
		K[] keys = this.keys;
		V[] values = this.values;
		StringBuilder buffer = new StringBuilder(32);
		buffer.append('{');
		buffer.append(keys[0]);
		buffer.append('=');
		buffer.append(values[0]);
		for (int i = 1; i < size; i++) {
			buffer.append(", ");
			buffer.append(keys[i]);
			buffer.append('=');
			buffer.append(values[i]);
		}
		buffer.append('}');
		return buffer.toString();
	}

	public Iterator<Entry<K, V>> iterator () {
		return entries();
	}

	
	public Entries<K, V> entries () {
		if (entries1 == null) {
			entries1 = new Entries(this);
			entries2 = new Entries(this);
		}
		if (!entries1.valid) {
			entries1.index = 0;
			entries1.valid = true;
			entries2.valid = false;
			return entries1;
		}
		entries2.index = 0;
		entries2.valid = true;
		entries1.valid = false;
		return entries2;
	}

	
	public Values<V> values () {
		if (valuesIter1 == null) {
			valuesIter1 = new Values(this);
			valuesIter2 = new Values(this);
		}
		if (!valuesIter1.valid) {
			valuesIter1.index = 0;
			valuesIter1.valid = true;
			valuesIter2.valid = false;
			return valuesIter1;
		}
		valuesIter2.index = 0;
		valuesIter2.valid = true;
		valuesIter1.valid = false;
		return valuesIter2;
	}

	
	public Keys<K> keys () {
		if (keysIter1 == null) {
			keysIter1 = new Keys(this);
			keysIter2 = new Keys(this);
		}
		if (!keysIter1.valid) {
			keysIter1.index = 0;
			keysIter1.valid = true;
			keysIter2.valid = false;
			return keysIter1;
		}
		keysIter2.index = 0;
		keysIter2.valid = true;
		keysIter1.valid = false;
		return keysIter2;
	}

	static public class Entries<K, V> implements Iterable<Entry<K, V>>, Iterator<Entry<K, V>> {
		private final ArrayMap<K, V> map;
		Entry<K, V> entry = new Entry();
		int index;
		boolean valid = true;

		public Entries (ArrayMap<K, V> map) {
			this.map = map;
		}

		public boolean hasNext () {
			if (!valid) throw new GdxRuntimeException("#iterator() cannot be used nested.");
			return index < map.size;
		}

		public Iterator<Entry<K, V>> iterator () {
			return this;
		}

		
		public Entry<K, V> next () {
			if (index >= map.size) throw new NoSuchElementException(String.valueOf(index));
			if (!valid) throw new GdxRuntimeException("#iterator() cannot be used nested.");
			entry.key = map.keys[index];
			entry.value = map.values[index++];
			return entry;
		}

		public void remove () {
			index--;
			map.removeIndex(index);
		}

		public void reset () {
			index = 0;
		}
	}

	static public class Values<V> implements Iterable<V>, Iterator<V> {
		private final ArrayMap<Object, V> map;
		int index;
		boolean valid = true;

		public Values (ArrayMap<Object, V> map) {
			this.map = map;
		}

		public boolean hasNext () {
			if (!valid) throw new GdxRuntimeException("#iterator() cannot be used nested.");
			return index < map.size;
		}

		public Iterator<V> iterator () {
			return this;
		}

		public V next () {
			if (index >= map.size) throw new NoSuchElementException(String.valueOf(index));
			if (!valid) throw new GdxRuntimeException("#iterator() cannot be used nested.");
			return map.values[index++];
		}

		public void remove () {
			index--;
			map.removeIndex(index);
		}

		public void reset () {
			index = 0;
		}

		public Array<V> toArray () {
			return new Array(true, map.values, index, map.size - index);
		}

		public Array<V> toArray (Array array) {
			array.addAll(map.values, index, map.size - index);
			return array;
		}
	}

	static public class Keys<K> implements Iterable<K>, Iterator<K> {
		private final ArrayMap<K, Object> map;
		int index;
		boolean valid = true;

		public Keys (ArrayMap<K, Object> map) {
			this.map = map;
		}

		public boolean hasNext () {
			if (!valid) throw new GdxRuntimeException("#iterator() cannot be used nested.");
			return index < map.size;
		}

		public Iterator<K> iterator () {
			return this;
		}

		public K next () {
			if (index >= map.size) throw new NoSuchElementException(String.valueOf(index));
			if (!valid) throw new GdxRuntimeException("#iterator() cannot be used nested.");
			return map.keys[index++];
		}

		public void remove () {
			index--;
			map.removeIndex(index);
		}

		public void reset () {
			index = 0;
		}

		public Array<K> toArray () {
			return new Array(true, map.keys, index, map.size - index);
		}

		public Array<K> toArray (Array array) {
			array.addAll(map.keys, index, map.size - index);
			return array;
		}
	}
}
