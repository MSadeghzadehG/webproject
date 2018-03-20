

package com.google.common.collect.testing.features;

import com.google.common.annotations.GwtCompatible;
import com.google.common.collect.testing.Helpers;
import java.lang.annotation.Inherited;
import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.util.Collection;
import java.util.LinkedHashSet;
import java.util.Set;
import java.util.SortedSet;


@SuppressWarnings("unchecked")
@GwtCompatible
public enum CollectionFeature implements Feature<Collection> {
  
  ALLOWS_NULL_QUERIES,
  ALLOWS_NULL_VALUES(ALLOWS_NULL_QUERIES),

  
  RESTRICTS_ELEMENTS,

  
  KNOWN_ORDER,

  
  NON_STANDARD_TOSTRING,

  
  REJECTS_DUPLICATES_AT_CREATION,

  SUPPORTS_ADD,
  SUPPORTS_REMOVE,
  SUPPORTS_ITERATOR_REMOVE,
  FAILS_FAST_ON_CONCURRENT_MODIFICATION,

  
  GENERAL_PURPOSE(SUPPORTS_ADD, SUPPORTS_REMOVE, SUPPORTS_ITERATOR_REMOVE),

  
  REMOVE_OPERATIONS(SUPPORTS_REMOVE, SUPPORTS_ITERATOR_REMOVE),

  SERIALIZABLE,
  SERIALIZABLE_INCLUDING_VIEWS(SERIALIZABLE),

  SUBSET_VIEW,
  DESCENDING_VIEW,

  
  NONE;

  private final Set<Feature<? super Collection>> implied;

  CollectionFeature(Feature<? super Collection>... implied) {
    this.implied = Helpers.copyToSet(implied);
  }

  @Override
  public Set<Feature<? super Collection>> getImpliedFeatures() {
    return implied;
  }

  @Retention(RetentionPolicy.RUNTIME)
  @Inherited
  @TesterAnnotation
  public @interface Require {
    CollectionFeature[] value() default {};

    CollectionFeature[] absent() default {};
  }
}
