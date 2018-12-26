

package com.google.common.collect.testing;

import com.google.common.annotations.GwtCompatible;
import java.util.Collections;
import java.util.EnumSet;
import java.util.Iterator;
import java.util.ListIterator;
import java.util.Set;


@GwtCompatible
public enum IteratorFeature {
  
  SUPPORTS_REMOVE,
  
  SUPPORTS_ADD,
  
  SUPPORTS_SET;

  
  public static final Set<IteratorFeature> UNMODIFIABLE = Collections.emptySet();

  
  public static final Set<IteratorFeature> MODIFIABLE =
      Collections.unmodifiableSet(EnumSet.allOf(IteratorFeature.class));
}
