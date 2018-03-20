

package com.badlogic.gdx.utils;

import com.badlogic.gdx.math.MathUtils;

import java.util.Arrays;


public class ByteArray {
	public byte[] items;
	public int size;
	public boolean ordered;

	
	public ByteArray () {
		this(true, 16);
	}

	
	public ByteArray (int capacity) {
		this(true, capacity);
	}

	
	public ByteArray (boolean ordered, int capacity) {
		this.ordered = ordered;
		items = new byte[capacity];
	}

	
	public ByteArray (ByteArray array) {
		this.ordered = array.ordered;
		size = array.size;
		items = new byte[size];
		System.arraycopy(array.items, 0, items, 0, size);
	}

	
	public ByteArray (byte[] array) {
		this(true, array, 0, array.length);
	}

	
	public ByteArray (boolean ordered, byte[] array, int startIndex, int count) {
		this(ordered, count);
		size = count;
		System.arraycopy(array, startIndex, items, 0, count);
	}

	public void add (byte value) {
		byte[] items = this.items;
		if (size == items.length) items = resize(Math.max(8, (int)(size * 1.75f)));
		items[size++] = value;
	}

	public void add (byte value1, byte value2) {
		byte[] items = this.items;
		if (size + 1 >= items.length) items = resize(Math.max(8, (int)(size * 1.75f)));
		items[size] = value1;
		items[size + 1] = value2;
		size += 2;
	}

	public void add (byte value1, byte value2, byte value3) {
		byte[] items = this.items;
		if (size + 2 >= items.length) items = resize(Math.max(8, (int)(size * 1.75f)));
		items[size] = value1;
		items[size + 1] = value2;
		items[size + 2] = value3;
		size += 3;
	}

	public void add (byte value1, byte value2, byte value3, byte value4) {
		byte[] items = this.items;
		if (size + 3 >= items.length) items = resize(Math.max(8, (int)(size * 1.8f))); 		items[size] = value1;
		items[size + 1] = value2;
		items[size + 2] = value3;
		items[size + 3] = value4;
		size += 4;
	}

	public void addAll (ByteArray array) {
		addAll(array, 0, array.size);
	}

	public void addAll (ByteArray array, int offset, int length) {
		if (offset + length > array.size)
			throw new IllegalArgumentException("offset + length must be <= size: " + offset + " + " + length + " <= " + array.size);
		addAll(array.items, offset, length);
	}

	public void addAll (byte... array) {
		addAll(array, 0, array.length);
	}

	public void addAll (byte[] array, int offset, int length) {
		byte[] items = this.items;
		int sizeNeeded = size + length;
		if (sizeNeeded > items.length) items = resize(Math.max(8, (int)(sizeNeeded * 1.75f)));
		System.arraycopy(array, offset, items, size, length);
		size += length;
	}

	public byte get (int index) {
		if (index >= size) throw new IndexOutOfBoundsException("index can't be >= size: " + index + " >= " + size);
		return items[index];
	}

	public void set (int index, byte value) {
		if (index >= size) throw new IndexOutOfBoundsException("index can't be >= size: " + index + " >= " + size);
		items[index] = value;
	}

	public void incr (int index, byte value) {
		if (index >= size) throw new IndexOutOfBoundsException("index can't be >= size: " + index + " >= " + size);
		items[index] += value;
	}

	public void mul (int index, byte value) {
		if (index >= size) throw new IndexOutOfBoundsException("index can't be >= size: " + index + " >= " + size);
		items[index] *= value;
	}

	public void insert (int index, byte value) {
		if (index > size) throw new IndexOutOfBoundsException("index can't be > size: " + index + " > " + size);
		byte[] items = this.items;
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
		byte[] items = this.items;
		byte firstValue = items[first];
		items[first] = items[second];
		items[second] = firstValue;
	}

	public boolean contains (byte value) {
		int i = size - 1;
		byte[] items = this.items;
		while (i >= 0)
			if (items[i--] == value) return true;
		return false;
	}

	public int indexOf (byte value) {
		byte[] items = this.items;
		for (int i = 0, n = size; i < n; i++)
			if (items[i] == value) return i;
		return -1;
	}

	public int lastIndexOf (byte value) {
		byte[] items = this.items;
		for (int i = size - 1; i >= 0; i--)
			if (items[i] == value) return i;
		return -1;
	}

	public boolean removeValue (byte value) {
		byte[] items = this.items;
		for (int i = 0, n = size; i < n; i++) {
			if (items[i] == value) {
				removeIndex(i);
				return true;
			}
		}
		return false;
	}

	
	public int removeIndex (int index) {
		if (index >= size) throw new IndexOutOfBoundsException("index can't be >= size: " + index + " >= " + size);
		byte[] items = this.items;
		int value = items[index];
		size--;
		if (ordered)
			System.arraycopy(items, index + 1, items, index, size - index);
		else
			items[index] = items[size];
		return value;
	}

	
	public void removeRange (int start, int end) {
		if (end >= size) throw new IndexOutOfBoundsException("end can't be >= size: " + end + " >= " + size);
		if (start > end) throw new IndexOutOfBoundsException("start can't be > end: " + start + " > " + end);
		byte[] items = this.items;
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

	
	public boolean removeAll (ByteArray array) {
		int size = this.size;
		int startSize = size;
		byte[] items = this.items;
		for (int i = 0, n = array.size; i < n; i++) {
			int item = array.get(i);
			for (int ii = 0; ii < size; ii++) {
				if (item == items[ii]) {
					removeIndex(ii);
					size--;
					break;
				}
			}
		}
		return size != startSize;
	}

	
	public byte pop () {
		return items[--size];
	}

	
	public byte peek () {
		return items[size - 1];
	}

	
	public byte first () {
		if (size == 0) throw new IllegalStateException("Array is empty.");
		return items[0];
	}

	public void clear () {
		size = 0;
	}

	
	public byte[] shrink () {
		if (items.length != size) resize(size);
		return items;
	}

	
	public byte[] ensureCapacity (int additionalCapacity) {
		int sizeNeeded = size + additionalCapacity;
		if (sizeNeeded > items.length) resize(Math.max(8, sizeNeeded));
		return items;
	}

	
	public byte[] setSize (int newSize) {
		if (newSize > items.length) resize(Math.max(8, newSize));
		size = newSize;
		return items;
	}

	protected byte[] resize (int newSize) {
		byte[] newItems = new byte[newSize];
		byte[] items = this.items;
		System.arraycopy(items, 0, newItems, 0, Math.min(size, newItems.length));
		this.items = newItems;
		return newItems;
	}

	public void sort () {
		Arrays.sort(items, 0, size);
	}

	public void reverse () {
		byte[] items = this.items;
		for (int i = 0, lastIndex = size - 1, n = size / 2; i < n; i++) {
			int ii = lastIndex - i;
			byte temp = items[i];
			items[i] = items[ii];
			items[ii] = temp;
		}
	}

	public void shuffle () {
		byte[] items = this.items;
		for (int i = size - 1; i >= 0; i--) {
			int ii = MathUtils.random(i);
			byte temp = items[i];
			items[i] = items[ii];
			items[ii] = temp;
		}
	}

	
	public void truncate (int newSize) {
		if (size > newSize) size = newSize;
	}

	
	public byte random () {
		if (size == 0) return 0;
		return items[MathUtils.random(0, size - 1)];
	}

	public byte[] toArray () {
		byte[] array = new byte[size];
		System.arraycopy(items, 0, array, 0, size);
		return array;
	}

	public int hashCode () {
		if (!ordered) return super.hashCode();
		byte[] items = this.items;
		int h = 1;
		for (int i = 0, n = size; i < n; i++)
			h = h * 31 + items[i];
		return h;
	}

	public boolean equals (Object object) {
		if (object == this) return true;
		if (!ordered) return false;
		if (!(object instanceof ByteArray)) return false;
		ByteArray array = (ByteArray)object;
		if (!array.ordered) return false;
		int n = size;
		if (n != array.size) return false;
		byte[] items1 = this.items;
		byte[] items2 = array.items;
		for (int i = 0; i < n; i++)
			if (items1[i] != items2[i]) return false;
		return true;
	}

	public String toString () {
		if (size == 0) return "[]";
		byte[] items = this.items;
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
		byte[] items = this.items;
		StringBuilder buffer = new StringBuilder(32);
		buffer.append(items[0]);
		for (int i = 1; i < size; i++) {
			buffer.append(separator);
			buffer.append(items[i]);
		}
		return buffer.toString();
	}

	
	static public ByteArray with (byte... array) {
		return new ByteArray(array);
	}
}