
package com.iluwatar.flyweight;

import org.junit.jupiter.api.Test;

import java.util.ArrayList;
import java.util.List;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertNotNull;


public class AlchemistShopTest {

  @Test
  public void testShop() throws Exception {
    final AlchemistShop shop = new AlchemistShop();

    final List<Potion> bottomShelf = shop.getBottomShelf();
    assertNotNull(bottomShelf);
    assertEquals(5, bottomShelf.size());

    final List<Potion> topShelf = shop.getTopShelf();
    assertNotNull(topShelf);
    assertEquals(8, topShelf.size());

    final List<Potion> allPotions = new ArrayList<>();
    allPotions.addAll(topShelf);
    allPotions.addAll(bottomShelf);

        assertEquals(13, allPotions.size());
    assertEquals(5, allPotions.stream().map(System::identityHashCode).distinct().count());

  }

}
