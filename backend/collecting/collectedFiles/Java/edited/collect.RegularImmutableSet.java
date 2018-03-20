

package com.google.common.collect;

import java.util.Collections;
import java.util.Set;


final class RegularImmutableSet<E> extends ForwardingImmutableSet<E> {
  static final RegularImmutableSet<Object> EMPTY =
      new RegularImmutableSet<Object>(Collections.emptySet());

  RegularImmutableSet(Set<E> delegate) {
    super(delegate);
  }
}
