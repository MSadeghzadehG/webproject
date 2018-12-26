

package com.google.common.collect.testing.features;

import com.google.common.annotations.GwtCompatible;
import com.google.common.collect.testing.Helpers;
import java.lang.annotation.Inherited;
import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.util.List;
import java.util.Set;


@SuppressWarnings("unchecked")
@GwtCompatible
public enum ListFeature implements Feature<List> {
  SUPPORTS_SET,
  SUPPORTS_ADD_WITH_INDEX(CollectionFeature.SUPPORTS_ADD),
  SUPPORTS_REMOVE_WITH_INDEX(CollectionFeature.SUPPORTS_REMOVE),

  GENERAL_PURPOSE(
      CollectionFeature.GENERAL_PURPOSE,
      SUPPORTS_SET,
      SUPPORTS_ADD_WITH_INDEX,
      SUPPORTS_REMOVE_WITH_INDEX),

  
  REMOVE_OPERATIONS(CollectionFeature.REMOVE_OPERATIONS, SUPPORTS_REMOVE_WITH_INDEX);

  private final Set<Feature<? super List>> implied;

  ListFeature(Feature<? super List>... implied) {
    this.implied = Helpers.copyToSet(implied);
  }

  @Override
  public Set<Feature<? super List>> getImpliedFeatures() {
    return implied;
  }

  @Retention(RetentionPolicy.RUNTIME)
  @Inherited
  @TesterAnnotation
  public @interface Require {
    ListFeature[] value() default {};

    ListFeature[] absent() default {};
  }
}
