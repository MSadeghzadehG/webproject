

package com.badlogic.gdx.utils;

import java.util.Comparator;
import java.util.Iterator;
import java.util.NoSuchElementException;

import com.badlogic.gdx.math.MathUtils;
import com.badlogic.gdx.utils.reflect.ArrayReflection;


public class Array<T> implements Iterable<T> {
	
	public T[] items;

	public int size;
	public boolean ordered;

	private ArrayIterable iterable;
	private Predicate.PredicateIterable<T> predicateIterable;

	
	public Array () {
		this(true, 16);
	}

	
	public Array (int capacity) {
		this(true, capacity);
	}

	
	public Array (boolean ordered, int capacity) {
		this.ordered = ordered;
		items = (T[])new Object[capacity];
	}

	
	public Array (boolean ordered, int capacity, Class arrayType) {
		this.ordered = ordered;
		items = (T[])ArrayReflection.newInstance(arrayType, capacity);
	}

	
	public Array (Class arrayType) {
		this(true, 16, arrayType);
	}

	
	public Array (Array<? extends T> array) {
		this(array.ordered, array.size, array.items.getClass().getComponentType());
		size = array.size;
		System.arraycopy(array.items, 0, items, 0, size);
	}

	
	public Array (T[] array) {
		this(true, array, 0, array.length);
	}

	
	public Array (boolean ordered, T[] array, int start, int count) {
		this(ordered, count, (Class)array.getClass().getComponentType());
		size = count;
		System.arraycopy(array, start, items, 0, size);
	}

	public void add (T value) {
		T[] items = this.items;
		if (size == items.length) items = resize(Math.max(8, (int)(size * 1.75f)));
		items[size++] = value;
	}

	public void add (T value1, T value2) {
		T[] items = this.items;
		if (size + 1 >= items.length) items = resize(Math.max(8, (int)(size * 1.75f)));
		items[size] = value1;
		items[size + 1] = value2;
		size += 2;
	}

	public void add (T value1, T value2, T value3) {
		T[] items = this.items;
		if (size + 2 >= items.length) items = resize(Math.max(8, (int)(size * 1.75f)));
		items[size] = value1;
		items[size + 1] = value2;
		items[size + 2] = value3;
		size += 3;
	}

	public void add (T value1, T value2, T value3, T value4) {
		T[] items = this.items;
		if (size + 3 >= items.length) items = resize(Math.max(8, (int)(size * 1.8f))); 		items[size] = value1;
		items[size + 1] = value2;
		items[size + 2] = value3;
		items[size + 3] = value4;
		size += 4;
	}

	public void addAll (Array<? extends T> array) {
		addAll(array, 0, array.size);
	}

	public void addAll (Array<? extends T> array, int start, int count) {
		if (start + count > array.size)
			throw new IllegalArgumentException("start + count must be <= size: " + start + " + " + count + " <= " + array.size);
		addAll((T[])array.items, start, count);
	}

	public void addAll (T... array) {
		addAll(array, 0, array.length);
	}

	public void addAll (T[] array, int start, int count) {
		T[] items = this.items;
		int sizeNeeded = size + count;
		if (sizeNeeded > items.length) items = resize(Math.max(8, (int)(sizeNeeded * 1.75f)));
		System.arraycopy(array, start, items, size, count);
		size += count;
	}

	public T get (int index) {
		if (index >= size) throw new IndexOutOfBoundsException("index can't be >= size: " + index + " >= " + size);
		return items[index];
	}

	public void set (int index, T value) {
		if (index >= size) throw new IndexOutOfBoundsException("index can't be >= size: " + index + " >= " + size);
		items[index] = value;
	}

	public void insert (int index, T value) {
		if (index > size) throw new IndexOutOfBoundsException("index can't be > size: " + index + " > " + size);
		T[] items = this.items;
		if (size == items.length) items = resize(Math.max(8, (int)(size * 1.75f)));
		if (ordered)
			System.arraycopy(items, index, items, index + 1, size - index);
		else
			items[size] = items[index];
		size++;
		items[index] = value;
	}

	public void swap (int first, int second) {
		if (first >= size) throw new IndexOutOfBoundsException("first can't be >= size: " + first + " >= " + size);
		if (second >= size) throw new IndexOutOfBoundsException("second can't be >= size: " + second + " >= " + size);
		T[] items = this.items;
		T firstValue = items[first];
		items[first] = items[second];
		items[second] = firstValue;
	}

	
	public boolean contains (T value, boolean identity) {
		T[] items = this.items;
		int i = size - 1;
		if (identity || value == null) {
			while (i >= 0)
				if (items[i--] == value) return true;
		} else {
			while (i >= 0)
				if (value.equals(items[i--])) return true;
		}
		return false;
	}

	
	public int indexOf (T value, boolean identity) {
		T[] items = this.items;
		if (identity || value == null) {
			for (int i = 0, n = size; i < n; i++)
				if (items[i] == value) return i;
		} else {
			for (int i = 0, n = size; i < n; i++)
				if (value.equals(items[i])) return i;
		}
		return -1;
	}

	
	public int lastIndexOf (T value, boolean identity) {
		T[] items = this.items;
		if (identity || value == null) {
			for (int i = size - 1; i >= 0; i--)
				if (items[i] == value) return i;
		} else {
			for (int i = size - 1; i >= 0; i--)
				if (value.equals(items[i])) return i;
		}
		return -1;
	}

	
	public boolean removeValue (T value, boolean identity) {
		T[] items = this.items;
		if (identity || value == null) {
			for (int i = 0, n = size; i < n; i++) {
				if (items[i] == value) {
					removeIndex(i);
					return true;
				}
			}
		} else {
			for (int i = 0, n = size; i < n; i++) {
				if (value.equals(items[i])) {
					removeIndex(i);
					return true;
				}
			}
		}
		return false;
	}

	
	public T removeIndex (int index) {
		if (index >= size) throw new IndexOutOfBoundsException("index can't be >= size: " + index + " >= " + size);
		T[] items = this.items;
		T value = (T)items[index];
		size--;
		if (ordered)
			System.arraycopy(items, index + 1, items, index, size - index);
		else
			items[index] = items[size];
		items[size] = null;
		return value;
	}

	
	public void removeRange (int start, int end) {
		if (end >= size) throw new IndexOutOfBoundsException("end can't be >= size: " + end + " >= " + size);
		if (start > end) throw new IndexOutOfBoundsException("start can't be > end: " + start + " > " + end);
		T[] items = this.items;
		int count = end - start + 1;
		if (ordered)
			System.arraycopy(items, start + count, items, start, size - (start + count));
		else {
			int lastIndex = this.size - 1;
			for (int i = 0; i < count; i++)
				items[start + i] = items[lastIndex - i];
		}
		size -= count;
	}

	
	public boolean removeAll (Array<? extends T> array, boolean identity) {
		int size = this.size;
		int startSize = size;
		T[] items = this.items;
		if (identity) {
			for (int i = 0, n = array.size; i < n; i++) {
				T item = array.get(i);
				for (int ii = 0; ii < size; ii++) {
					if (item == items[ii]) {
						removeIndex(ii);
						size--;
						break;
					}
				}
			}
		} else {
			for (int i = 0, n = array.size; i < n; i++) {
				T item = array.get(i);
				for (int ii = 0; ii < size; ii++) {
					if (item.equals(items[ii])) {
						removeIndex(ii);
						size--;
						break;
					}
				}
			}
		}
		return size != startSize;
	}

	
	public T pop () {
		if (size == 0) throw new IllegalStateException("Array is empty.");
		--size;
		T item = items[size];
		items[size] = null;
		return item;
	}

	
	public T peek () {
		if (size == 0) throw new IllegalStateException("Array is empty.");
		return items[size - 1];
	}

	
	public T first () {
		if (size == 0) throw new IllegalStateException("Array is empty.");
		return items[0];
	}

	public void clear () {
		T[] items = this.items;
		for (int i = 0, n = size; i < n; i++)
			items[i] = null;
		size = 0;
	}

	
	public T[] shrink () {
		if (items.length != size) resize(size);
		return items;
	}

	
	public T[] ensureCapacity (int additionalCapacity) {
		int sizeNeeded = size + additionalCapacity;
		if (sizeNeeded > items.length) resize(Math.max(8, sizeNeeded));
		return items;
	}

	
	public T[] setSize (int newSize) {
		truncate(newSize);
		if (newSize > items.length) resize(Math.max(8, newSize));
		size = newSize;
		return items;
	}

	
	protected T[] resize (int newSize) {
		T[] items = this.items;
		T[] newItems = (T[])ArrayReflection.newInstance(items.getClass().getComponentType(), newSize);
		System.arraycopy(items, 0, newItems, 0, Math.min(size, newItems.length));
		this.items = newItems;
		return newItems;
	}

	
	public void sort () {
		Sort.instance().sort(items, 0, size);
	}

	
	public void sort (Comparator<? super T> comparator) {
		Sort.instance().sort(items, comparator, 0, size);
	}

	
	public T selectRanked (Comparator<T> comparator, int kthLowest) {
		if (kthLowest < 1) {
			throw new GdxRuntimeException("nth_lowest must be greater than 0, 1 = first, 2 = second...");
		}
		return Select.instance().select(items, comparator, kthLowest, size);
	}

	
	public int selectRankedIndex (Comparator<T> comparator, int kthLowest) {
		if (kthLowest < 1) {
			throw new GdxRuntimeException("nth_lowest must be greater than 0, 1 = first, 2 = second...");
		}
		return Select.instance().selectIndex(items, comparator, kthLowest, size);
	}

	public void reverse () {
		T[] items = this.items;
		for (int i = 0, lastIndex = size - 1, n = size / 2; i < n; i++) {
			int ii = lastIndex - i;
			T temp = items[i];
			items[i] = items[ii];
			items[ii] = temp;
		}
	}

	public void shuffle () {
		T[] items = this.items;
		for (int i = size - 1; i >= 0; i--) {
			int ii = MathUtils.random(i);
			T temp = items[i];
			items[i] = items[ii];
			items[ii] = temp;
		}
	}

	
	public Iterator<T> iterator () {
		if (iterable == null) iterable = new ArrayIterable(this);
		return iterable.iterator();
	}

	
	public Iterable<T> select (Predicate<T> predicate) {
		if (predicateIterable == null)
			predicateIterable = new Predicate.PredicateIterable<T>(this, predicate);
		else
			predicateIterable.set(this, predicate);
		return predicateIterable;
	}

	
	public void truncate (int newSize) {
		if (size <= newSize) return;
		for (int i = newSize; i < size; i++)
			items[i] = null;
		size = newSize;
	}

	
	public T random () {
		if (size == 0) return null;
		return items[MathUtils.random(0, size - 1)];
	}

	
	public T[] toArray () {
		return (T[])toArray(items.getClass().getComponentType());
	}

	public <V> V[] toArray (Class type) {
		V[] result = (V[])ArrayReflection.newInstance(type, size);
		System.arraycopy(items, 0, result, 0, size);
		return result;
	}

	public int hashCode () {
		if (!ordered) return super.hashCode();
		Object[] items = this.items;
		int h = 1;
		for (int i = 0, n = size; i < n; i++) {
			h *= 31;
			Object item = items[i];
			if (item != null) h += item.hashCode();
		}
		return h;
	}

	public boolean equals (Object object) {
		if (object == this) return true;
		if (!ordered) return false;
		if (!(object instanceof Array)) return false;
		Array array = (Array)object;
		if (!array.ordered) return false;
		int n = size;
		if (n != array.size) return false;
		Object[] items1 = this.items;
		Object[] items2 = array.items;
		for (int i = 0; i < n; i++) {
			Object o1 = items1[i];
			Object o2 = items2[i];
			if (!(o1 == null ? o2 == null : o1.equals(o2))) return false;
		}
		return true;
	}

	public String toString () {
		if (size == 0) return "[]";
		T[] items = this.items;
		StringBuilder buffer = new StringBuilder(32);
		buffer.append('[');
		buffer.append(items[0]);
		for (int i = 1; i < size; i++) {
			buffer.append(", ");
			buffer.append(items[i]);
		}
		buffer.append(']');
		return buffer.toString();
	}

	public String toString (String separator) {
		if (size == 0) return "";
		T[] items = this.items;
		StringBuilder buffer = new StringBuilder(32);
		buffer.append(items[0]);
		for (int i = 1; i < size; i++) {
			buffer.append(separator);
			buffer.append(items[i]);
		}
		return buffer.toString();
	}

	
	static public <T> Array<T> of (Class<T> arrayType) {
		return new Array<T>(arrayType);
	}

	
	static public <T> Array<T> of (boolean ordered, int capacity, Class<T> arrayType) {
		return new Array<T>(ordered, capacity, arrayType);
	}

	
	static public <T> Array<T> with (T... array) {
		return new Array(array);
	}

	static public class ArrayIterator<T> implements Iterator<T>, Iterable<T> {
		private final Array<T> array;
		private final boolean allowRemove;
		int index;
		boolean valid = true;


		public ArrayIterator (Array<T> array) {
			this(array, true);
		}

		public ArrayIterator (Array<T> array, boolean allowRemove) {
			this.array = array;
			this.allowRemove = allowRemove;
		}

		public boolean hasNext () {
			if (!valid) {
				throw new GdxRuntimeException("#iterator() cannot be used nested.");
			}
			return index < array.size;
		}

		public T next () {
			if (index >= array.size) throw new NoSuchElementException(String.valueOf(index));
			if (!valid) {
				throw new GdxRuntimeException("#iterator() cannot be used nested.");
			}
			return array.items[index++];
		}

		public void remove () {
			if (!allowRemove) throw new GdxRuntimeException("Remove not allowed.");
			index--;
			array.removeIndex(index);
		}

		public void reset () {
			index = 0;
		}

		public Iterator<T> iterator () {
			return this;
		}
	}

	static public class ArrayIterable<T> implements Iterable<T> {
		private final Array<T> array;
		private final boolean allowRemove;
		private ArrayIterator iterator1, iterator2;


		public ArrayIterable (Array<T> array) {
			this(array, true);
		}

		public ArrayIterable (Array<T> array, boolean allowRemove) {
			this.array = array;
			this.allowRemove = allowRemove;
		}

		public Iterator<T> iterator () {
			if (iterator1 == null) {
				iterator1 = new ArrayIterator(array, allowRemove);
				iterator2 = new ArrayIterator(array, allowRemove);
			}
			if (!iterator1.valid) {
				iterator1.index = 0;
				iterator1.valid = true;
				iterator2.valid = false;
				return iterator1;
			}
			iterator2.index = 0;
			iterator2.valid = true;
			iterator1.valid = false;
			return iterator2;
		}
	}
}
