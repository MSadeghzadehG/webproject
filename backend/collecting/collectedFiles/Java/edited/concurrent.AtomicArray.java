

package org.elasticsearch.common.util.concurrent;

import org.elasticsearch.ElasticsearchGenerationException;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.concurrent.atomic.AtomicReferenceArray;


public class AtomicArray<E> {
    private final AtomicReferenceArray<E> array;
    private volatile List<E> nonNullList;

    public AtomicArray(int size) {
        array = new AtomicReferenceArray<>(size);
    }

    
    public int length() {
        return array.length();
    }

    
    public void set(int i, E value) {
        array.set(i, value);
        if (nonNullList != null) {             nonNullList = null;
        }
    }

    public final void setOnce(int i, E value) {
        if (array.compareAndSet(i, null, value) == false) {
            throw new IllegalStateException("index [" + i + "] has already been set");
        }
        if (nonNullList != null) {             nonNullList = null;
        }
    }

    
    public E get(int i) {
        return array.get(i);
    }

    
    public List<E> asList() {
        if (nonNullList == null) {
            if (array == null || array.length() == 0) {
                nonNullList = Collections.emptyList();
            } else {
                List<E> list = new ArrayList<>(array.length());
                for (int i = 0; i < array.length(); i++) {
                    E e = array.get(i);
                    if (e != null) {
                        list.add(e);
                    }
                }
                nonNullList = list;
            }
        }
        return nonNullList;
    }

    
    public E[] toArray(E[] a) {
        if (a.length != array.length()) {
            throw new ElasticsearchGenerationException("AtomicArrays can only be copied to arrays of the same size");
        }
        for (int i = 0; i < array.length(); i++) {
            a[i] = array.get(i);
        }
        return a;
    }
}
