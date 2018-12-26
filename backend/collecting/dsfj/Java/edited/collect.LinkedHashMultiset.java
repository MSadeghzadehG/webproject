

package com.google.common.collect;

import com.google.common.annotations.GwtCompatible;
import com.google.common.annotations.GwtIncompatible;
import java.io.IOException;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.util.LinkedHashMap;


@GwtCompatible(serializable = true, emulated = true)
@SuppressWarnings("serial") public final class LinkedHashMultiset<E> extends AbstractMapBasedMultiset<E> {

  
  public static <E> LinkedHashMultiset<E> create() {
    return new LinkedHashMultiset<E>();
  }

  
  public static <E> LinkedHashMultiset<E> create(int distinctElements) {
    return new LinkedHashMultiset<E>(distinctElements);
  }

  
  public static <E> LinkedHashMultiset<E> create(Iterable<? extends E> elements) {
    LinkedHashMultiset<E> multiset = create(Multisets.inferDistinctElements(elements));
    Iterables.addAll(multiset, elements);
    return multiset;
  }

  private LinkedHashMultiset() {
    super(new LinkedHashMap<E, Count>());
  }

  private LinkedHashMultiset(int distinctElements) {
    super(Maps.<E, Count>newLinkedHashMapWithExpectedSize(distinctElements));
  }

  
  @GwtIncompatible   private void writeObject(ObjectOutputStream stream) throws IOException {
    stream.defaultWriteObject();
    Serialization.writeMultiset(this, stream);
  }

  @GwtIncompatible   private void readObject(ObjectInputStream stream) throws IOException, ClassNotFoundException {
    stream.defaultReadObject();
    int distinctElements = Serialization.readCount(stream);
    setBackingMap(new LinkedHashMap<E, Count>());
    Serialization.populateMultiset(this, stream, distinctElements);
  }

  @GwtIncompatible   private static final long serialVersionUID = 0;
}
