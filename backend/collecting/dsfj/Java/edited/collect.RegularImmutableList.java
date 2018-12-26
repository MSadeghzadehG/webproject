

package com.google.common.collect;

import static java.util.Collections.unmodifiableList;

import java.util.List;


class RegularImmutableList<E> extends ForwardingImmutableList<E> {
  private final List<E> delegate;
  E forSerialization;

  RegularImmutableList(List<E> delegate) {
        this.delegate = unmodifiableList(delegate);
  }

  @Override
  List<E> delegateList() {
    return delegate;
  }
}
