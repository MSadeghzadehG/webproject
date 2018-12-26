

package com.badlogic.gdx.utils;

import java.util.Iterator;
import java.util.NoSuchElementException;

import com.badlogic.gdx.math.MathUtils;


public class IntFloatMap implements Iterable<IntFloatMap.Entry> {
	private static final int PRIME1 = 0xbe1f14b1;
	private static final int PRIME2 = 0xb4b82e39;
	private static final int PRIME3 = 0xced1c241;
	private static final int EMPTY = 0;

	public int size;

	int[] keyTable;
	float[] valueTable;
	int capacity, stashSize;
	float zeroValue;
	boolean hasZeroValue;

	private float loadFactor;
	private int hashShift, mask, threshold;
	private int stashCapacity;
	private int pushIterations;

	private Entries entries1, entries2;
	private Values values1, values2;
	private Keys keys1, keys2;

	
	public IntFloatMap () {
		this(51, 0.8f);
	}

	
	public IntFloatMap (int initialCapacity) {
		this(initialCapacity, 0.8f);
	}

	
	public IntFloatMap (int initialCapacity, float loadFactor) {
		if (initialCapacity < 0) throw new IllegalArgumentException("initialCapacity must be >= 0: " + initialCapacity);
		initialCapacity = MathUtils.nextPowerOfTwo((int)Math.ceil(initialCapacity / loadFactor));
		if (initialCapacity > 1 << 30) throw new IllegalArgumentException("initialCapacity is too large: " + initialCapacity);
		capacity = initialCapacity;

		if (loadFactor <= 0) throw new IllegalArgumentException("loadFactor must be > 0: " + loadFactor);
		this.loadFactor = loadFactor;

		threshold = (int)(capacity * loadFactor);
		mask = capacity - 1;
		hashShift = 31 - Integer.numberOfTrailingZeros(capacity);
		stashCapacity = Math.max(3, (int)Math.ceil(Math.log(capacity)) * 2);
		pushIterations = Math.max(Math.min(capacity, 8), (int)Math.sqrt(capacity) / 8);

		keyTable = new int[capacity + stashCapacity];
		valueTable = new float[keyTable.length];
	}

	
	public IntFloatMap (IntFloatMap map) {
		this((int)Math.floor(map.capacity * map.loadFactor), map.loadFactor);
		stashSize = map.stashSize;
		System.arraycopy(map.keyTable, 0, keyTable, 0, map.keyTable.length);
		System.arraycopy(map.valueTable, 0, valueTable, 0, map.valueTable.length);
		size = map.size;
		zeroValue = map.zeroValue;
		hasZeroValue = map.hasZeroValue;
	}

	public void put (int key, float value) {
		if (key == 0) {
			zeroValue = value;
			if (!hasZeroValue) {
				hasZeroValue = true;
				size++;
			}
			return;
		}

		int[] keyTable = this.keyTable;

				int index1 = key & mask;
		int key1 = keyTable[index1];
		if (key == key1) {
			valueTable[index1] = value;
			return;
		}

		int index2 = hash2(key);
		int key2 = keyTable[index2];
		if (key == key2) {
			valueTable[index2] = value;
			return;
		}

		int index3 = hash3(key);
		int key3 = keyTable[index3];
		if (key == key3) {
			valueTable[index3] = value;
			return;
		}

				for (int i = capacity, n = i + stashSize; i < n; i++) {
			if (key == keyTable[i]) {
				valueTable[i] = value;
				return;
			}
		}

				if (key1 == EMPTY) {
			keyTable[index1] = key;
			valueTable[index1] = value;
			if (size++ >= threshold) resize(capacity << 1);
			return;
		}

		if (key2 == EMPTY) {
			keyTable[index2] = key;
			valueTable[index2] = value;
			if (size++ >= threshold) resize(capacity << 1);
			return;
		}

		if (key3 == EMPTY) {
			keyTable[index3] = key;
			valueTable[index3] = value;
			if (size++ >= threshold) resize(capacity << 1);
			return;
		}

		push(key, value, index1, key1, index2, key2, index3, key3);
	}

	public void putAll (IntFloatMap map) {
		for (Entry entry : map.entries())
			put(entry.key, entry.value);
	}

	
	private void putResize (int key, float value) {
		if (key == 0) {
			zeroValue = value;
			hasZeroValue = true;
			return;
		}

				int index1 = key & mask;
		int key1 = keyTable[index1];
		if (key1 == EMPTY) {
			keyTable[index1] = key;
			valueTable[index1] = value;
			if (size++ >= threshold) resize(capacity << 1);
			return;
		}

		int index2 = hash2(key);
		int key2 = keyTable[index2];
		if (key2 == EMPTY) {
			keyTable[index2] = key;
			valueTable[index2] = value;
			if (size++ >= threshold) resize(capacity << 1);
			return;
		}

		int index3 = hash3(key);
		int key3 = keyTable[index3];
		if (key3 == EMPTY) {
			keyTable[index3] = key;
			valueTable[index3] = value;
			if (size++ >= threshold) resize(capacity << 1);
			return;
		}

		push(key, value, index1, key1, index2, key2, index3, key3);
	}

	private void push (int insertKey, float insertValue, int index1, int key1, int index2, int key2, int index3, int key3) {
		int[] keyTable = this.keyTable;
		float[] valueTable = this.valueTable;
		int mask = this.mask;

				int evictedKey;
		float evictedValue;
		int i = 0, pushIterations = this.pushIterations;
		do {
						switch (MathUtils.random(2)) {
			case 0:
				evictedKey = key1;
				evictedValue = valueTable[index1];
				keyTable[index1] = insertKey;
				valueTable[index1] = insertValue;
				break;
			case 1:
				evictedKey = key2;
				evictedValue = valueTable[index2];
				keyTable[index2] = insertKey;
				valueTable[index2] = insertValue;
				break;
			default:
				evictedKey = key3;
				evictedValue = valueTable[index3];
				keyTable[index3] = insertKey;
				valueTable[index3] = insertValue;
				break;
			}

						index1 = evictedKey & mask;
			key1 = keyTable[index1];
			if (key1 == EMPTY) {
				keyTable[index1] = evictedKey;
				valueTable[index1] = evictedValue;
				if (size++ >= threshold) resize(capacity << 1);
				return;
			}

			index2 = hash2(evictedKey);
			key2 = keyTable[index2];
			if (key2 == EMPTY) {
				keyTable[index2] = evictedKey;
				valueTable[index2] = evictedValue;
				if (size++ >= threshold) resize(capacity << 1);
				return;
			}

			index3 = hash3(evictedKey);
			key3 = keyTable[index3];
			if (key3 == EMPTY) {
				keyTable[index3] = evictedKey;
				valueTable[index3] = evictedValue;
				if (size++ >= threshold) resize(capacity << 1);
				return;
			}

			if (++i == pushIterations) break;

			insertKey = evictedKey;
			insertValue = evictedValue;
		} while (true);

		putStash(evictedKey, evictedValue);
	}

	private void putStash (int key, float value) {
		if (stashSize == stashCapacity) {
						resize(capacity << 1);
			put(key, value);
			return;
		}
				int index = capacity + stashSize;
		keyTable[index] = key;
		valueTable[index] = value;
		stashSize++;
		size++;
	}

	
	public float get (int key, float defaultValue) {
		if (key == 0) {
			if (!hasZeroValue) return defaultValue;
			return zeroValue;
		}
		int index = key & mask;
		if (keyTable[index] != key) {
			index = hash2(key);
			if (keyTable[index] != key) {
				index = hash3(key);
				if (keyTable[index] != key) return getStash(key, defaultValue);
			}
		}
		return valueTable[index];
	}

	private float getStash (int key, float defaultValue) {
		int[] keyTable = this.keyTable;
		for (int i = capacity, n = i + stashSize; i < n; i++)
			if (key == keyTable[i]) return valueTable[i];
		return defaultValue;
	}

	
	public float getAndIncrement (int key, float defaultValue, float increment) {
		if (key == 0) {
			if (hasZeroValue) {
				float value = zeroValue;
				zeroValue += increment;
				return value;
			} else {
				hasZeroValue = true;
				zeroValue = defaultValue + increment;
				++size;
				return defaultValue;
			}
		}
		int index = key & mask;
		if (key != keyTable[index]) {
			index = hash2(key);
			if (key != keyTable[index]) {
				index = hash3(key);
				if (key != keyTable[index]) return getAndIncrementStash(key, defaultValue, increment);
			}
		}
		float value = valueTable[index];
		valueTable[index] = value + increment;
		return value;
	}

	private float getAndIncrementStash (int key, float defaultValue, float increment) {
		int[] keyTable = this.keyTable;
		for (int i = capacity, n = i + stashSize; i < n; i++)
			if (key == keyTable[i]) {
				float value = valueTable[i];
				valueTable[i] = value + increment;
				return value;
			}
		put(key, defaultValue + increment);
		return defaultValue;
	}

	public float remove (int key, float defaultValue) {
		if (key == 0) {
			if (!hasZeroValue) return defaultValue;
			hasZeroValue = false;
			size--;
			return zeroValue;
		}

		int index = key & mask;
		if (key == keyTable[index]) {
			keyTable[index] = EMPTY;
			float oldValue = valueTable[index];
			size--;
			return oldValue;
		}

		index = hash2(key);
		if (key == keyTable[index]) {
			keyTable[index] = EMPTY;
			float oldValue = valueTable[index];
			size--;
			return oldValue;
		}

		index = hash3(key);
		if (key == keyTable[index]) {
			keyTable[index] = EMPTY;
			float oldValue = valueTable[index];
			size--;
			return oldValue;
		}

		return removeStash(key, defaultValue);
	}

	float removeStash (int key, float defaultValue) {
		int[] keyTable = this.keyTable;
		for (int i = capacity, n = i + stashSize; i < n; i++) {
			if (key == keyTable[i]) {
				float oldValue = valueTable[i];
				removeStashIndex(i);
				size--;
				return oldValue;
			}
		}
		return defaultValue;
	}

	void removeStashIndex (int index) {
				stashSize--;
		int lastIndex = capacity + stashSize;
		if (index < lastIndex) {
			keyTable[index] = keyTable[lastIndex];
			valueTable[index] = valueTable[lastIndex];
		}
	}

	
	public void shrink (int maximumCapacity) {
		if (maximumCapacity < 0) throw new IllegalArgumentException("maximumCapacity must be >= 0: " + maximumCapacity);
		if (size > maximumCapacity) maximumCapacity = size;
		if (capacity <= maximumCapacity) return;
		maximumCapacity = MathUtils.nextPowerOfTwo(maximumCapacity);
		resize(maximumCapacity);
	}

	
	public void clear (int maximumCapacity) {
		if (capacity <= maximumCapacity) {
			clear();
			return;
		}
		hasZeroValue = false;
		size = 0;
		resize(maximumCapacity);
	}

	public void clear () {
		if (size == 0) return;
		int[] keyTable = this.keyTable;
		for (int i = capacity + stashSize; i-- > 0;)
			keyTable[i] = EMPTY;
		hasZeroValue = false;
		size = 0;
		stashSize = 0;
	}

	
	public boolean containsValue (float value) {
		if (hasZeroValue && zeroValue == value) return true;
		int[] keyTable = this.keyTable;
		float[] valueTable = this.valueTable;
		for (int i = capacity + stashSize; i-- > 0;)
			if (keyTable[i] != 0 && valueTable[i] == value) return true;
		return false;
	}

	
	public boolean containsValue (float value, float epsilon) {
		if (hasZeroValue && Math.abs(zeroValue - value) <= epsilon) return true;
		float[] valueTable = this.valueTable;
		for (int i = capacity + stashSize; i-- > 0;)
			if (Math.abs(valueTable[i] - value) <= epsilon) return true;
		return false;
	}

	public boolean containsKey (int key) {
		if (key == 0) return hasZeroValue;
		int index = key & mask;
		if (keyTable[index] != key) {
			index = hash2(key);
			if (keyTable[index] != key) {
				index = hash3(key);
				if (keyTable[index] != key) return containsKeyStash(key);
			}
		}
		return true;
	}

	private boolean containsKeyStash (int key) {
		int[] keyTable = this.keyTable;
		for (int i = capacity, n = i + stashSize; i < n; i++)
			if (key == keyTable[i]) return true;
		return false;
	}

	
	public int findKey (float value, int notFound) {
		if (hasZeroValue && zeroValue == value) return 0;
		int[] keyTable = this.keyTable;
		float[] valueTable = this.valueTable;
		for (int i = capacity + stashSize; i-- > 0;)
			if (keyTable[i] != 0 && valueTable[i] == value) return keyTable[i];
		return notFound;
	}

	
	public void ensureCapacity (int additionalCapacity) {
		int sizeNeeded = size + additionalCapacity;
		if (sizeNeeded >= threshold) resize(MathUtils.nextPowerOfTwo((int)Math.ceil(sizeNeeded / loadFactor)));
	}

	private void resize (int newSize) {
		int oldEndIndex = capacity + stashSize;

		capacity = newSize;
		threshold = (int)(newSize * loadFactor);
		mask = newSize - 1;
		hashShift = 31 - Integer.numberOfTrailingZeros(newSize);
		stashCapacity = Math.max(3, (int)Math.ceil(Math.log(newSize)) * 2);
		pushIterations = Math.max(Math.min(newSize, 8), (int)Math.sqrt(newSize) / 8);

		int[] oldKeyTable = keyTable;
		float[] oldValueTable = valueTable;

		keyTable = new int[newSize + stashCapacity];
		valueTable = new float[newSize + stashCapacity];

		int oldSize = size;
		size = hasZeroValue ? 1 : 0;
		stashSize = 0;
		if (oldSize > 0) {
			for (int i = 0; i < oldEndIndex; i++) {
				int key = oldKeyTable[i];
				if (key != EMPTY) putResize(key, oldValueTable[i]);
			}
		}
	}

	private int hash2 (int h) {
		h *= PRIME2;
		return (h ^ h >>> hashShift) & mask;
	}

	private int hash3 (int h) {
		h *= PRIME3;
		return (h ^ h >>> hashShift) & mask;
	}

	public int hashCode () {
		int h = 0;
		if (hasZeroValue) {
			h += Float.floatToIntBits(zeroValue);
		}
		int[] keyTable = this.keyTable;
		float[] valueTable = this.valueTable;
		for (int i = 0, n = capacity + stashSize; i < n; i++) {
			int key = keyTable[i];
			if (key != EMPTY) {
				h += key * 31;

				float value = valueTable[i];
				h += Float.floatToIntBits(value);
			}
		}
		return h;
	}

	public boolean equals (Object obj) {
		if (obj == this) return true;
		if (!(obj instanceof IntFloatMap)) return false;
		IntFloatMap other = (IntFloatMap)obj;
		if (other.size != size) return false;
		if (other.hasZeroValue != hasZeroValue) return false;
		if (hasZeroValue && other.zeroValue != zeroValue) {
			return false;
		}
		int[] keyTable = this.keyTable;
		float[] valueTable = this.valueTable;
		for (int i = 0, n = capacity + stashSize; i < n; i++) {
			int key = keyTable[i];
			if (key != EMPTY) {
				float otherValue = other.get(key, 0f);
				if (otherValue == 0f && !other.containsKey(key)) return false;
				float value = valueTable[i];
				if (otherValue != value) return false;
			}
		}
		return true;
	}

	public String toString () {
		if (size == 0) return "{}";
		StringBuilder buffer = new StringBuilder(32);
		buffer.append('{');
		int[] keyTable = this.keyTable;
		float[] valueTable = this.valueTable;
		int i = keyTable.length;
		if (hasZeroValue) {
			buffer.append("0=");
			buffer.append(zeroValue);
		} else {
			while (i-- > 0) {
				int key = keyTable[i];
				if (key == EMPTY) continue;
				buffer.append(key);
				buffer.append('=');
				buffer.append(valueTable[i]);
				break;
			}
		}
		while (i-- > 0) {
			int key = keyTable[i];
			if (key == EMPTY) continue;
			buffer.append(", ");
			buffer.append(key);
			buffer.append('=');
			buffer.append(valueTable[i]);
		}
		buffer.append('}');
		return buffer.toString();
	}

	public Iterator<Entry> iterator () {
		return entries();
	}

	
	public Entries entries () {
		if (entries1 == null) {
			entries1 = new Entries(this);
			entries2 = new Entries(this);
		}
		if (!entries1.valid) {
			entries1.reset();
			entries1.valid = true;
			entries2.valid = false;
			return entries1;
		}
		entries2.reset();
		entries2.valid = true;
		entries1.valid = false;
		return entries2;
	}

	
	public Values values () {
		if (values1 == null) {
			values1 = new Values(this);
			values2 = new Values(this);
		}
		if (!values1.valid) {
			values1.reset();
			values1.valid = true;
			values2.valid = false;
			return values1;
		}
		values2.reset();
		values2.valid = true;
		values1.valid = false;
		return values2;
	}

	
	public Keys keys () {
		if (keys1 == null) {
			keys1 = new Keys(this);
			keys2 = new Keys(this);
		}
		if (!keys1.valid) {
			keys1.reset();
			keys1.valid = true;
			keys2.valid = false;
			return keys1;
		}
		keys2.reset();
		keys2.valid = true;
		keys1.valid = false;
		return keys2;
	}

	static public class Entry {
		public int key;
		public float value;

		public String toString () {
			return key + "=" + value;
		}
	}

	static private class MapIterator {
		static final int INDEX_ILLEGAL = -2;
		static final int INDEX_ZERO = -1;

		public boolean hasNext;

		final IntFloatMap map;
		int nextIndex, currentIndex;
		boolean valid = true;

		public MapIterator (IntFloatMap map) {
			this.map = map;
			reset();
		}

		public void reset () {
			currentIndex = INDEX_ILLEGAL;
			nextIndex = INDEX_ZERO;
			if (map.hasZeroValue)
				hasNext = true;
			else
				findNextIndex();
		}

		void findNextIndex () {
			hasNext = false;
			int[] keyTable = map.keyTable;
			for (int n = map.capacity + map.stashSize; ++nextIndex < n;) {
				if (keyTable[nextIndex] != EMPTY) {
					hasNext = true;
					break;
				}
			}
		}

		public void remove () {
			if (currentIndex == INDEX_ZERO && map.hasZeroValue) {
				map.hasZeroValue = false;
			} else if (currentIndex < 0) {
				throw new IllegalStateException("next must be called before remove.");
			} else if (currentIndex >= map.capacity) {
				map.removeStashIndex(currentIndex);
				nextIndex = currentIndex - 1;
				findNextIndex();
			} else {
				map.keyTable[currentIndex] = EMPTY;
			}
			currentIndex = INDEX_ILLEGAL;
			map.size--;
		}
	}

	static public class Entries extends MapIterator implements Iterable<Entry>, Iterator<Entry> {
		private Entry entry = new Entry();

		public Entries (IntFloatMap map) {
			super(map);
		}

		
		public Entry next () {
			if (!hasNext) throw new NoSuchElementException();
			if (!valid) throw new GdxRuntimeException("#iterator() cannot be used nested.");
			int[] keyTable = map.keyTable;
			if (nextIndex == INDEX_ZERO) {
				entry.key = 0;
				entry.value = map.zeroValue;
			} else {
				entry.key = keyTable[nextIndex];
				entry.value = map.valueTable[nextIndex];
			}
			currentIndex = nextIndex;
			findNextIndex();
			return entry;
		}

		public boolean hasNext () {
			if (!valid) throw new GdxRuntimeException("#iterator() cannot be used nested.");
			return hasNext;
		}

		public Iterator<Entry> iterator () {
			return this;
		}

		public void remove () {
			super.remove();
		}
	}

	static public class Values extends MapIterator {
		public Values (IntFloatMap map) {
			super(map);
		}

		public boolean hasNext () {
			if (!valid) throw new GdxRuntimeException("#iterator() cannot be used nested.");
			return hasNext;
		}

		public float next () {
			if (!hasNext) throw new NoSuchElementException();
			if (!valid) throw new GdxRuntimeException("#iterator() cannot be used nested.");
			float value;
			if (nextIndex == INDEX_ZERO)
				value = map.zeroValue;
			else
				value = map.valueTable[nextIndex];
			currentIndex = nextIndex;
			findNextIndex();
			return value;
		}

		
		public FloatArray toArray () {
			FloatArray array = new FloatArray(true, map.size);
			while (hasNext)
				array.add(next());
			return array;
		}
	}

	static public class Keys extends MapIterator {
		public Keys (IntFloatMap map) {
			super(map);
		}

		public boolean hasNext () {
			if (!valid) throw new GdxRuntimeException("#iterator() cannot be used nested.");
			return hasNext;
		}

		public int next () {
			if (!hasNext) throw new NoSuchElementException();
			if (!valid) throw new GdxRuntimeException("#iterator() cannot be used nested.");
			int key = nextIndex == INDEX_ZERO ? 0 : map.keyTable[nextIndex];
			currentIndex = nextIndex;
			findNextIndex();
			return key;
		}

		
		public IntArray toArray () {
			IntArray array = new IntArray(true, map.size);
			while (hasNext)
				array.add(next());
			return array;
		}
	}
}
