

package com.google.common.collect.testing.features;

import com.google.common.annotations.GwtCompatible;
import com.google.common.collect.testing.Helpers;
import java.lang.annotation.Inherited;
import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.util.Set;


@SuppressWarnings("unchecked")
@GwtCompatible
public enum SetFeature implements Feature<Set> {
  GENERAL_PURPOSE(CollectionFeature.GENERAL_PURPOSE);

  private final Set<Feature<? super Set>> implied;

  SetFeature(Feature<? super Set>... implied) {
    this.implied = Helpers.copyToSet(implied);
  }

  @Override
  public Set<Feature<? super Set>> getImpliedFeatures() {
    return implied;
  }

  @Retention(RetentionPolicy.RUNTIME)
  @Inherited
  @TesterAnnotation
  public @interface Require {
    public abstract SetFeature[] value() default {};

    public abstract SetFeature[] absent() default {};
  }
}
