
package com.iluwatar.prototype;

import org.junit.jupiter.params.ParameterizedTest;
import org.junit.jupiter.params.provider.MethodSource;

import java.util.Arrays;
import java.util.Collection;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertNotNull;
import static org.junit.jupiter.api.Assertions.assertNotSame;
import static org.junit.jupiter.api.Assertions.assertSame;


public class PrototypeTest<P extends Prototype> {
  static Collection<Object[]> dataProvider() {
    return Arrays.asList(
            new Object[]{new OrcBeast("axe"), "Orcish wolf attacks with axe"},
            new Object[]{new OrcMage("sword"), "Orcish mage attacks with sword"},
            new Object[]{new OrcWarlord("laser"), "Orcish warlord attacks with laser"},
            new Object[]{new ElfBeast("cooking"), "Elven eagle helps in cooking"},
            new Object[]{new ElfMage("cleaning"), "Elven mage helps in cleaning"},
            new Object[]{new ElfWarlord("protecting"), "Elven warlord helps in protecting"}
    );
  }

  @ParameterizedTest
  @MethodSource("dataProvider")
  public void testPrototype(P testedPrototype, String expectedToString) throws Exception {
    assertEquals(expectedToString, testedPrototype.toString());

    final Object clone = testedPrototype.copy();
    assertNotNull(clone);
    assertNotSame(clone, testedPrototype);
    assertSame(testedPrototype.getClass(), clone.getClass());
  }

}
