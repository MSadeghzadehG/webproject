
package io.reactivex.internal.util;

import java.util.*;


public class LinkedArrayList {
    
    final int capacityHint;
    
    Object[] head;
    
    Object[] tail;
    
    volatile int size;
    
    int indexInTail;
    
    public LinkedArrayList(int capacityHint) {
        this.capacityHint = capacityHint;
    }
    
    public void add(Object o) {
                if (size == 0) {
            head = new Object[capacityHint + 1];
            tail = head;
            head[0] = o;
            indexInTail = 1;
            size = 1;
        } else
                if (indexInTail == capacityHint) {
            Object[] t = new Object[capacityHint + 1];
            t[0] = o;
            tail[capacityHint] = t;
            tail = t;
            indexInTail = 1;
            size++;
        } else {
            tail[indexInTail] = o;
            indexInTail++;
            size++;
        }
    }
    
    public Object[] head() {
        return head;     }

    
    public int size() {
        return size;
    }
    @Override
    public String toString() {
        final int cap = capacityHint;
        final int s = size;
        final List<Object> list = new ArrayList<Object>(s + 1);

        Object[] h = head();
        int j = 0;
        int k = 0;
        while (j < s) {
            list.add(h[k]);
            j++;
            if (++k == cap) {
                k = 0;
                h = (Object[])h[cap];
            }
        }

        return list.toString();
    }
}
