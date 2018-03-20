

package com.google.common.collect.testing.features;

import com.google.common.annotations.GwtCompatible;
import com.google.common.collect.testing.Helpers;
import java.lang.annotation.Inherited;
import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.util.Collection;
import java.util.Collections;
import java.util.Set;


@SuppressWarnings("unchecked")
@GwtCompatible
public enum CollectionSize implements Feature<Collection>, Comparable<CollectionSize> {
  
  ZERO(0),
  
  ONE(1),
  
  SEVERAL(3),
  

  ANY(ZERO, ONE, SEVERAL);

  private final Set<Feature<? super Collection>> implied;
  private final Integer numElements;

  CollectionSize(int numElements) {
    this.implied = Collections.emptySet();
    this.numElements = numElements;
  }

  CollectionSize(Feature<? super Collection>... implied) {
            this.implied = Helpers.copyToSet(implied);
    this.numElements = null;
  }

  @Override
  public Set<Feature<? super Collection>> getImpliedFeatures() {
    return implied;
  }

  public int getNumElements() {
    if (numElements == null) {
      throw new IllegalStateException(
          "A compound CollectionSize doesn't specify a number of elements.");
    }
    return numElements;
  }

  @Retention(RetentionPolicy.RUNTIME)
  @Inherited
  @TesterAnnotation
  public @interface Require {
    CollectionSize[] value() default {};

    CollectionSize[] absent() default {};
  }
}
