

package com.google.common.collect.testing.features;

import com.google.common.annotations.GwtCompatible;
import com.google.common.collect.testing.Helpers;
import java.util.Collections;
import java.util.Set;


@GwtCompatible
public final class TesterRequirements {
  private final Set<Feature<?>> presentFeatures;
  private final Set<Feature<?>> absentFeatures;

  public TesterRequirements(Set<Feature<?>> presentFeatures, Set<Feature<?>> absentFeatures) {
    this.presentFeatures = Helpers.copyToSet(presentFeatures);
    this.absentFeatures = Helpers.copyToSet(absentFeatures);
  }

  public TesterRequirements(TesterRequirements tr) {
    this(tr.getPresentFeatures(), tr.getAbsentFeatures());
  }

  public TesterRequirements() {
    this(Collections.<Feature<?>>emptySet(), Collections.<Feature<?>>emptySet());
  }

  public final Set<Feature<?>> getPresentFeatures() {
    return presentFeatures;
  }

  public final Set<Feature<?>> getAbsentFeatures() {
    return absentFeatures;
  }

  @Override
  public boolean equals(Object object) {
    if (object == this) {
      return true;
    }
    if (object instanceof TesterRequirements) {
      TesterRequirements that = (TesterRequirements) object;
      return this.presentFeatures.equals(that.presentFeatures)
          && this.absentFeatures.equals(that.absentFeatures);
    }
    return false;
  }

  @Override
  public int hashCode() {
    return presentFeatures.hashCode() * 31 + absentFeatures.hashCode();
  }

  @Override
  public String toString() {
    return "{TesterRequirements: present=" + presentFeatures + ", absent=" + absentFeatures + "}";
  }

  private static final long serialVersionUID = 0;
}
