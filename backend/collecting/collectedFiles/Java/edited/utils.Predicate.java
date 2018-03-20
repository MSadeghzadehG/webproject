

package com.badlogic.gdx.utils;

import java.util.Iterator;

import com.badlogic.gdx.utils.Array.ArrayIterator;


public interface Predicate<T> {

	
	boolean evaluate (T arg0);

	public class PredicateIterator<T> implements Iterator<T> {
		public Iterator<T> iterator;
		public Predicate<T> predicate;
		public boolean end = false;
		public boolean peeked = false;
		public T next = null;

		public PredicateIterator (final Iterable<T> iterable, final Predicate<T> predicate) {
			this(iterable.iterator(), predicate);
		}

		public PredicateIterator (final Iterator<T> iterator, final Predicate<T> predicate) {
			set(iterator, predicate);
		}

		public void set (final Iterable<T> iterable, final Predicate<T> predicate) {
			set(iterable.iterator(), predicate);
		}

		public void set (final Iterator<T> iterator, final Predicate<T> predicate) {
			this.iterator = iterator;
			this.predicate = predicate;
			end = peeked = false;
			next = null;
		}

		@Override
		public boolean hasNext () {
			if (end) return false;
			if (next != null) return true;
			peeked = true;
			while (iterator.hasNext()) {
				final T n = iterator.next();
				if (predicate.evaluate(n)) {
					next = n;
					return true;
				}
			}
			end = true;
			return false;
		}

		@Override
		public T next () {
			if (next == null && !hasNext()) return null;
			final T result = next;
			next = null;
			peeked = false;
			return result;
		}

		@Override
		public void remove () {
			if (peeked) throw new GdxRuntimeException("Cannot remove between a call to hasNext() and next().");
			iterator.remove();
		}
	}

	public static class PredicateIterable<T> implements Iterable<T> {
		public Iterable<T> iterable;
		public Predicate<T> predicate;
		public PredicateIterator<T> iterator = null;

		public PredicateIterable (Iterable<T> iterable, Predicate<T> predicate) {
			set(iterable, predicate);
		}

		public void set (Iterable<T> iterable, Predicate<T> predicate) {
			this.iterable = iterable;
			this.predicate = predicate;
		}

		
		@Override
		public Iterator<T> iterator () {
			if (iterator == null)
				iterator = new PredicateIterator<T>(iterable.iterator(), predicate);
			else
				iterator.set(iterable.iterator(), predicate);
			return iterator;
		}
	}
}
