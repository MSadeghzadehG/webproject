

package com.google.common.collect.testing.google;

import com.google.common.annotations.GwtCompatible;
import com.google.common.collect.Multiset;
import com.google.common.collect.testing.features.Feature;
import com.google.common.collect.testing.features.TesterAnnotation;
import java.lang.annotation.Inherited;
import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.util.Collections;
import java.util.Set;


@GwtCompatible
public enum MultisetFeature implements Feature<Multiset> {
  
  ENTRIES_ARE_VIEWS;

  @Override
  public Set<Feature<? super Multiset>> getImpliedFeatures() {
    return Collections.emptySet();
  }

  @Retention(RetentionPolicy.RUNTIME)
  @Inherited
  @TesterAnnotation
  public @interface Require {
    public abstract MultisetFeature[] value() default {};

    public abstract MultisetFeature[] absent() default {};
  }
}
