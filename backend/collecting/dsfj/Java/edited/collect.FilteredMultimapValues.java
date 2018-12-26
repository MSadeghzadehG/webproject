

package com.google.common.collect;

import static com.google.common.base.Preconditions.checkNotNull;

import com.google.common.annotations.GwtCompatible;
import com.google.common.base.Objects;
import com.google.common.base.Predicate;
import com.google.common.base.Predicates;
import com.google.j2objc.annotations.Weak;
import java.util.AbstractCollection;
import java.util.Collection;
import java.util.Iterator;
import java.util.Map.Entry;
import org.checkerframework.checker.nullness.compatqual.NullableDecl;


@GwtCompatible
final class FilteredMultimapValues<K, V> extends AbstractCollection<V> {
  @Weak private final FilteredMultimap<K, V> multimap;

  FilteredMultimapValues(FilteredMultimap<K, V> multimap) {
    this.multimap = checkNotNull(multimap);
  }

  @Override
  public Iterator<V> iterator() {
    return Maps.valueIterator(multimap.entries().iterator());
  }

  @Override
  public boolean contains(@NullableDecl Object o) {
    return multimap.containsValue(o);
  }

  @Override
  public int size() {
    return multimap.size();
  }

  @Override
  public boolean remove(@NullableDecl Object o) {
    Predicate<? super Entry<K, V>> entryPredicate = multimap.entryPredicate();
    for (Iterator<Entry<K, V>> unfilteredItr = multimap.unfiltered().entries().iterator();
        unfilteredItr.hasNext(); ) {
      Entry<K, V> entry = unfilteredItr.next();
      if (entryPredicate.apply(entry) && Objects.equal(entry.getValue(), o)) {
        unfilteredItr.remove();
        return true;
      }
    }
    return false;
  }

  @Override
  public boolean removeAll(Collection<?> c) {
    return Iterables.removeIf(
        multimap.unfiltered().entries(),
                Predicates.<Entry<K, V>>and(
            multimap.entryPredicate(), Maps.<V>valuePredicateOnEntries(Predicates.in(c))));
  }

  @Override
  public boolean retainAll(Collection<?> c) {
    return Iterables.removeIf(
        multimap.unfiltered().entries(),
                Predicates.<Entry<K, V>>and(
            multimap.entryPredicate(),
            Maps.<V>valuePredicateOnEntries(Predicates.not(Predicates.in(c)))));
  }

  @Override
  public void clear() {
    multimap.clear();
  }
}
