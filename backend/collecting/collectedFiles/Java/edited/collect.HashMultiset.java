

package com.google.common.collect;

import static com.google.common.base.Preconditions.checkArgument;
import static com.google.common.collect.CollectPreconditions.checkNonnegative;

import com.google.common.annotations.GwtCompatible;
import com.google.common.annotations.GwtIncompatible;
import com.google.common.primitives.Ints;
import com.google.errorprone.annotations.CanIgnoreReturnValue;
import java.io.IOException;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.io.Serializable;
import java.util.ConcurrentModificationException;
import java.util.Iterator;
import java.util.NoSuchElementException;
import org.checkerframework.checker.nullness.compatqual.NullableDecl;


@GwtCompatible(serializable = true, emulated = true)
public class HashMultiset<E> extends AbstractMapBasedMultiset<E> {

  
  public static <E> HashMultiset<E> create() {
    return create(ObjectCountHashMap.DEFAULT_SIZE);
  }

  
  public static <E> HashMultiset<E> create(int distinctElements) {
    return new HashMultiset<E>(distinctElements);
  }

  
  public static <E> HashMultiset<E> create(Iterable<? extends E> elements) {
    HashMultiset<E> multiset = create(Multisets.inferDistinctElements(elements));
    Iterables.addAll(multiset, elements);
    return multiset;
  }

  HashMultiset(int distinctElements) {
    super(distinctElements);
  }

  @Override
  void init(int distinctElements) {
    backingMap = new ObjectCountHashMap<>(distinctElements);
  }

  @GwtIncompatible   private static final long serialVersionUID = 0;
}
