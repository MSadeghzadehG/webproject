

package com.google.common.collect.testing.features;

import com.google.common.annotations.GwtCompatible;
import com.google.common.collect.testing.Helpers;
import java.lang.annotation.Inherited;
import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.util.Map;
import java.util.Set;


@SuppressWarnings("unchecked")
@GwtCompatible
public enum MapFeature implements Feature<Map> {
  
  ALLOWS_NULL_KEY_QUERIES,
  ALLOWS_NULL_KEYS(ALLOWS_NULL_KEY_QUERIES),
  
  ALLOWS_NULL_VALUE_QUERIES,
  ALLOWS_NULL_VALUES(ALLOWS_NULL_VALUE_QUERIES),
  
  ALLOWS_NULL_ENTRY_QUERIES,
  
  ALLOWS_ANY_NULL_QUERIES(
      ALLOWS_NULL_ENTRY_QUERIES, ALLOWS_NULL_KEY_QUERIES, ALLOWS_NULL_VALUE_QUERIES),
  RESTRICTS_KEYS,
  RESTRICTS_VALUES,
  SUPPORTS_PUT,
  SUPPORTS_REMOVE,
  FAILS_FAST_ON_CONCURRENT_MODIFICATION,
  
  REJECTS_DUPLICATES_AT_CREATION,

  GENERAL_PURPOSE(SUPPORTS_PUT, SUPPORTS_REMOVE);

  private final Set<Feature<? super Map>> implied;

  MapFeature(Feature<? super Map>... implied) {
    this.implied = Helpers.copyToSet(implied);
  }

  @Override
  public Set<Feature<? super Map>> getImpliedFeatures() {
    return implied;
  }

  @Retention(RetentionPolicy.RUNTIME)
  @Inherited
  @TesterAnnotation
  public @interface Require {
    public abstract MapFeature[] value() default {};

    public abstract MapFeature[] absent() default {};
  }
}
