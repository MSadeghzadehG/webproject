

package com.google.common.collect.testing;

import com.google.common.annotations.GwtCompatible;
import java.util.ArrayList;
import java.util.List;
import java.util.ListIterator;


@GwtCompatible
public abstract class ListIteratorTester<E> extends AbstractIteratorTester<E, ListIterator<E>> {
  protected ListIteratorTester(
      int steps,
      Iterable<E> elementsToInsert,
      Iterable<? extends IteratorFeature> features,
      Iterable<E> expectedElements,
      int startIndex) {
    super(steps, elementsToInsert, features, expectedElements, KnownOrder.KNOWN_ORDER, startIndex);
  }

  @Override
  protected final Iterable<? extends Stimulus<E, ? super ListIterator<E>>> getStimulusValues() {
    List<Stimulus<E, ? super ListIterator<E>>> list = new ArrayList<>();
    Helpers.addAll(list, iteratorStimuli());
    Helpers.addAll(list, listIteratorStimuli());
    return list;
  }

  @Override
  protected abstract ListIterator<E> newTargetIterator();
}
