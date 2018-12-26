

package com.badlogic.gdx.graphics;

import java.util.Iterator;
import java.util.NoSuchElementException;

import com.badlogic.gdx.utils.GdxRuntimeException;


public final class VertexAttributes implements Iterable<VertexAttribute>, Comparable<VertexAttributes> {
	
	public static final class Usage {
		public static final int Position = 1;
		public static final int ColorUnpacked = 2;
		public static final int ColorPacked = 4;
		public static final int Normal = 8;
		public static final int TextureCoordinates = 16;
		public static final int Generic = 32;
		public static final int BoneWeight = 64;
		public static final int Tangent = 128;
		public static final int BiNormal = 256;
	}

	
	private final VertexAttribute[] attributes;

	
	public final int vertexSize;

	
	private long mask = -1;

	private ReadonlyIterable<VertexAttribute> iterable;

	
	public VertexAttributes (VertexAttribute... attributes) {
		if (attributes.length == 0) throw new IllegalArgumentException("attributes must be >= 1");

		VertexAttribute[] list = new VertexAttribute[attributes.length];
		for (int i = 0; i < attributes.length; i++)
			list[i] = attributes[i];

		this.attributes = list;
		vertexSize = calculateOffsets();
	}

	
	public int getOffset (int usage, int defaultIfNotFound) {
		VertexAttribute vertexAttribute = findByUsage(usage);
		if (vertexAttribute == null) return defaultIfNotFound;
		return vertexAttribute.offset / 4;
	}
	
	
	public int getOffset (int usage) {
		return getOffset(usage, 0);
	}

	
	public VertexAttribute findByUsage (int usage) {
		int len = size();
		for (int i = 0; i < len; i++)
			if (get(i).usage == usage) return get(i);
		return null;
	}

	private int calculateOffsets () {
		int count = 0;
		for (int i = 0; i < attributes.length; i++) {
			VertexAttribute attribute = attributes[i];
			attribute.offset = count;
			count += attribute.getSizeInBytes();
		}

		return count;
	}

	
	public int size () {
		return attributes.length;
	}

	
	public VertexAttribute get (int index) {
		return attributes[index];
	}

	public String toString () {
		StringBuilder builder = new StringBuilder();
		builder.append("[");
		for (int i = 0; i < attributes.length; i++) {
			builder.append("(");
			builder.append(attributes[i].alias);
			builder.append(", ");
			builder.append(attributes[i].usage);
			builder.append(", ");
			builder.append(attributes[i].numComponents);
			builder.append(", ");
			builder.append(attributes[i].offset);
			builder.append(")");
			builder.append("\n");
		}
		builder.append("]");
		return builder.toString();
	}

	@Override
	public boolean equals (final Object obj) {
		if (obj == this) return true;
		if (!(obj instanceof VertexAttributes)) return false;
		VertexAttributes other = (VertexAttributes)obj;
		if (this.attributes.length != other.attributes.length) return false;
		for (int i = 0; i < attributes.length; i++) {
			if (!attributes[i].equals(other.attributes[i])) return false;
		}
		return true;
	}

	@Override
	public int hashCode () {
		long result = 61 * attributes.length;
		for (int i = 0; i < attributes.length; i++)
			result = result * 61 + attributes[i].hashCode();
		return (int)(result ^ (result >> 32));
	}

	
	public long getMask () {
		if (mask == -1) {
			long result = 0;
			for (int i = 0; i < attributes.length; i++) {
				result |= attributes[i].usage;
			}
			mask = result;
		}
		return mask;
	}

	
	public long getMaskWithSizePacked () {
		return getMask() | ((long)attributes.length << 32);
	}

	@Override
	public int compareTo (VertexAttributes o) {
		if (attributes.length != o.attributes.length) return attributes.length - o.attributes.length;
		final long m1 = getMask();
		final long m2 = o.getMask();
		if (m1 != m2) return m1 < m2 ? -1 : 1;
		for (int i = attributes.length - 1; i >= 0; --i) {
			final VertexAttribute va0 = attributes[i];
			final VertexAttribute va1 = o.attributes[i];
			if (va0.usage != va1.usage) return va0.usage - va1.usage;
			if (va0.unit != va1.unit) return va0.unit - va1.unit;
			if (va0.numComponents != va1.numComponents) return va0.numComponents - va1.numComponents;
			if (va0.normalized != va1.normalized) return va0.normalized ? 1 : -1;
			if (va0.type != va1.type) return va0.type - va1.type;
		}
		return 0;
	}

	@Override
	public Iterator<VertexAttribute> iterator () {
		if (iterable == null) iterable = new ReadonlyIterable<VertexAttribute>(attributes);
		return iterable.iterator();
	}

	static private class ReadonlyIterator<T> implements Iterator<T>, Iterable<T> {
		private final T[] array;
		int index;
		boolean valid = true;

		public ReadonlyIterator (T[] array) {
			this.array = array;
		}

		@Override
		public boolean hasNext () {
			if (!valid) throw new GdxRuntimeException("#iterator() cannot be used nested.");
			return index < array.length;
		}

		@Override
		public T next () {
			if (index >= array.length) throw new NoSuchElementException(String.valueOf(index));
			if (!valid) throw new GdxRuntimeException("#iterator() cannot be used nested.");
			return array[index++];
		}

		@Override
		public void remove () {
			throw new GdxRuntimeException("Remove not allowed.");
		}

		public void reset () {
			index = 0;
		}

		@Override
		public Iterator<T> iterator () {
			return this;
		}
	}

	static private class ReadonlyIterable<T> implements Iterable<T> {
		private final T[] array;
		private ReadonlyIterator iterator1, iterator2;

		public ReadonlyIterable (T[] array) {
			this.array = array;
		}

		@Override
		public Iterator<T> iterator () {
			if (iterator1 == null) {
				iterator1 = new ReadonlyIterator(array);
				iterator2 = new ReadonlyIterator(array);
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
