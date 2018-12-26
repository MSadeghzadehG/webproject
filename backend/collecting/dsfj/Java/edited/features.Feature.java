

package com.google.common.collect.testing.features;

import com.google.common.annotations.GwtCompatible;
import java.util.Set;


@GwtCompatible
public interface Feature<T> {
  
  Set<Feature<? super T>> getImpliedFeatures();
}
