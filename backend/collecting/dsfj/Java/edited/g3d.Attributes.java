

package com.badlogic.gdx.graphics.g3d;

import java.util.Comparator;
import java.util.Iterator;

import com.badlogic.gdx.utils.Array;

public class Attributes implements Iterable<Attribute>, Comparator<Attribute>, Comparable<Attributes> {
	protected long mask;
	protected final Array<Attribute> attributes = new Array<Attribute>();

	protected boolean sorted = true;

	
	public final void sort () {
		if (!sorted) {
			attributes.sort(this);
			sorted = true;
		}
	}

	
	public final long getMask () {
		return mask;
	}

	
	public final Attribute get (final long type) {
		if (has(type)) for (int i = 0; i < attributes.size; i++)
			if (attributes.get(i).type == type) return attributes.get(i);
		return null;
	}

	
	public final <T extends Attribute> T get (Class<T> clazz, final long type) {
		return (T)get(type);
	}

	
	public final Array<Attribute> get (final Array<Attribute> out, final long type) {
		for (int i = 0; i < attributes.size; i++)
			if ((attributes.get(i).type & type) != 0) out.add(attributes.get(i));
		return out;
	}

	
	public void clear () {
		mask = 0;
		attributes.clear();
	}

	
	public int size () {
		return attributes.size;
	}

	private final void enable (final long mask) {
		this.mask |= mask;
	}

	private final void disable (final long mask) {
		this.mask &= ~mask;
	}

	
	public final void set (final Attribute attribute) {
		final int idx = indexOf(attribute.type);
		if (idx < 0) {
			enable(attribute.type);
			attributes.add(attribute);
			sorted = false;
		} else {
			attributes.set(idx, attribute);
		}
		sort(); 	}

	
	public final void set (final Attribute attribute1, final Attribute attribute2) {
		set(attribute1);
		set(attribute2);
	}

	
	public final void set (final Attribute attribute1, final Attribute attribute2, final Attribute attribute3) {
		set(attribute1);
		set(attribute2);
		set(attribute3);
	}

	
	public final void set (final Attribute attribute1, final Attribute attribute2, final Attribute attribute3,
		final Attribute attribute4) {
		set(attribute1);
		set(attribute2);
		set(attribute3);
		set(attribute4);
	}

	
	public final void set (final Attribute... attributes) {
		for (final Attribute attr : attributes)
			set(attr);
	}

	
	public final void set (final Iterable<Attribute> attributes) {
		for (final Attribute attr : attributes)
			set(attr);
	}

	
	public final void remove (final long mask) {
		for (int i = attributes.size - 1; i >= 0; i--) {
			final long type = attributes.get(i).type;
			if ((mask & type) == type) {
				attributes.removeIndex(i);
				disable(type);
				sorted = false;
			}
		}
		sort(); 	}

	
	public final boolean has (final long type) {
		return type != 0 && (this.mask & type) == type;
	}

	
	protected int indexOf (final long type) {
		if (has(type)) for (int i = 0; i < attributes.size; i++)
			if (attributes.get(i).type == type) return i;
		return -1;
	}

	
	public final boolean same (final Attributes other, boolean compareValues) {
		if (other == this) return true;
		if ((other == null) || (mask != other.mask)) return false;
		if (!compareValues) return true;
		sort();
		other.sort();
		for (int i = 0; i < attributes.size; i++)
			if (!attributes.get(i).equals(other.attributes.get(i))) return false;
		return true;
	}

	
	public final boolean same (final Attributes other) {
		return same(other, false);
	}

	
	@Override
	public final int compare (final Attribute arg0, final Attribute arg1) {
		return (int)(arg0.type - arg1.type);
	}

	
	@Override
	public final Iterator<Attribute> iterator () {
		return attributes.iterator();
	}

	
	public int attributesHash () {
		sort();
		final int n = attributes.size;
		long result = 71 + mask;
		int m = 1;
		for (int i = 0; i < n; i++)
			result += mask * attributes.get(i).hashCode() * (m = (m * 7) & 0xFFFF);
		return (int)(result ^ (result >> 32));
	}

	@Override
	public int hashCode () {
		return attributesHash();
	}

	@Override
	public boolean equals (Object other) {
		if (!(other instanceof Attributes)) return false;
		if (other == this) return true;
		return same((Attributes)other, true);
	}

	@Override
	public int compareTo (Attributes other) {
		if (other == this)
			return 0;
		if (mask != other.mask)
			return mask < other.mask ? -1 : 1;
		sort();
		other.sort();
		for (int i = 0; i < attributes.size; i++) {
			final int c = attributes.get(i).compareTo(other.attributes.get(i));
			if (c != 0)
				return c < 0 ? -1 : (c > 0 ? 1 : 0);
		}
		return 0;
	}
}
