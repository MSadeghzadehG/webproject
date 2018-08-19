
package com.iluwatar.doubledispatch;

import java.util.Objects;

import static org.junit.jupiter.api.Assertions.assertEquals;


public abstract class CollisionTest<O extends GameObject> {

  
  abstract O getTestedObject();

  
  void testCollision(final GameObject other, final boolean otherDamaged, final boolean otherOnFire,
                     final boolean thisDamaged, final boolean thisOnFire, final String description) {

    Objects.requireNonNull(other);
    Objects.requireNonNull(getTestedObject());

    final O tested = getTestedObject();

    tested.collision(other);

    testOnFire(other, tested, otherOnFire);
    testDamaged(other, tested, otherDamaged);

    testOnFire(tested, other, thisOnFire);
    testDamaged(tested, other, thisDamaged);

  }

  
  private void testOnFire(final GameObject target, final GameObject other, final boolean expectTargetOnFire) {
    final String targetName = target.getClass().getSimpleName();
    final String otherName = other.getClass().getSimpleName();

    final String errorMessage = expectTargetOnFire
        ? "Expected [" + targetName + "] to be on fire after colliding with [" + otherName + "] but it was not!"
        : "Expected [" + targetName + "] not to be on fire after colliding with [" + otherName + "] but it was!";

    assertEquals(expectTargetOnFire, target.isOnFire(), errorMessage);
  }

  
  private void testDamaged(final GameObject target, final GameObject other, final boolean expectedDamage) {
    final String targetName = target.getClass().getSimpleName();
    final String otherName = other.getClass().getSimpleName();

    final String errorMessage = expectedDamage
        ? "Expected [" + targetName + "] to be damaged after colliding with [" + otherName + "] but it was not!"
        : "Expected [" + targetName + "] not to be damaged after colliding with [" + otherName + "] but it was!";

    assertEquals(expectedDamage, target.isDamaged(), errorMessage);
  }

}
