

package com.badlogic.gdx.utils;


public class SortedIntList<E> implements Iterable<SortedIntList.Node<E>> {
	private NodePool<E> nodePool = new NodePool<E>(); 	private Iterator iterator;
	int size = 0;

	Node<E> first;

	
	public SortedIntList () {
	}

	
	public E insert (int index, E value) {
		if (first != null) {
			Node<E> c = first;
						while (c.n != null && c.n.index <= index) {
				c = c.n;
			}
						if (index > c.index) {
				c.n = nodePool.obtain(c, c.n, value, index);
				if (c.n.n != null) {
					c.n.n.p = c.n;
				}
				size++;
			}
						else if (index < c.index) {
				Node<E> newFirst = nodePool.obtain(null, first, value, index);
				first.p = newFirst;
				first = newFirst;
				size++;
			}
						else {
				c.value = value;
			}
		} else {
			first = nodePool.obtain(null, null, value, index);
			size++;
		}
		return null;
	}

	
	public E get (int index) {
		E match = null;
		if (first != null) {
			Node<E> c = first;
			while (c.n != null && c.index < index) {
				c = c.n;
			}
			if (c.index == index) {
				match = c.value;
			}
		}
		return match;
	}

	
	public void clear () {
		for (; first != null; first = first.n) {
			nodePool.free(first);
		}
		size = 0;
	}

	
	public int size () {
		return size;
	}

	
	public java.util.Iterator<Node<E>> iterator () {
		if (iterator == null) {
			iterator = new Iterator();
		}
		return iterator.reset();
	}

	class Iterator implements java.util.Iterator<Node<E>> {
		private Node<E> position;
		private Node<E> previousPosition;

		@Override
		public boolean hasNext () {
			return position != null;
		}

		@Override
		public Node<E> next () {
			previousPosition = position;
			position = position.n;
			return previousPosition;
		}

		@Override
		public void remove () {
						if (previousPosition != null) {
								if (previousPosition == first) {
					first = position;
				}
								else {
					previousPosition.p.n = position;
					if (position != null) {
						position.p = previousPosition.p;
					}
				}
				size--;
			}
		}

		public Iterator reset () {
			position = first;
			previousPosition = null;
			return this;
		}
	}

	public static class Node<E> {
		
		protected Node<E> p;
		
		protected Node<E> n;
		
		public E value;
		
		public int index;
	}

	static class NodePool<E> extends Pool<Node<E>> {
		@Override
		protected Node<E> newObject () {
			return new Node<E>();
		}

		public Node<E> obtain (Node<E> p, Node<E> n, E value, int index) {
			Node<E> newNode = super.obtain();
			newNode.p = p;
			newNode.n = n;
			newNode.value = value;
			newNode.index = index;
			return newNode;
		}
	}
}
