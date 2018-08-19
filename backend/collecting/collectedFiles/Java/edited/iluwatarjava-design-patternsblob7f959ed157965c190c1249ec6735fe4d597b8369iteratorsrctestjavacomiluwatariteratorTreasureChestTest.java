
package com.iluwatar.iterator;

import org.junit.jupiter.params.ParameterizedTest;
import org.junit.jupiter.params.provider.MethodSource;

import java.util.ArrayList;
import java.util.List;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertNotNull;
import static org.junit.jupiter.api.Assertions.fail;


public class TreasureChestTest {

  
  public static List<Object[]> dataProvider() {
    final List<Object[]> parameters = new ArrayList<>();
    parameters.add(new Object[]{new Item(ItemType.POTION, "Potion of courage")});
    parameters.add(new Object[]{new Item(ItemType.RING, "Ring of shadows")});
    parameters.add(new Object[]{new Item(ItemType.POTION, "Potion of wisdom")});
    parameters.add(new Object[]{new Item(ItemType.POTION, "Potion of blood")});
    parameters.add(new Object[]{new Item(ItemType.WEAPON, "Sword of silver +1")});
    parameters.add(new Object[]{new Item(ItemType.POTION, "Potion of rust")});
    parameters.add(new Object[]{new Item(ItemType.POTION, "Potion of healing")});
    parameters.add(new Object[]{new Item(ItemType.RING, "Ring of armor")});
    parameters.add(new Object[]{new Item(ItemType.WEAPON, "Steel halberd")});
    parameters.add(new Object[]{new Item(ItemType.WEAPON, "Dagger of poison")});
    return parameters;
  }

  
  @ParameterizedTest
  @MethodSource("dataProvider")
  public void testIterator(Item expectedItem) {
    final TreasureChest chest = new TreasureChest();
    final ItemIterator iterator = chest.iterator(expectedItem.getType());
    assertNotNull(iterator);

    while (iterator.hasNext()) {
      final Item item = iterator.next();
      assertNotNull(item);
      assertEquals(expectedItem.getType(), item.getType());

      final String name = item.toString();
      assertNotNull(name);
      if (expectedItem.toString().equals(name)) {
        return;
      }
    }

    fail("Expected to find item [" + expectedItem + "] using iterator, but we didn't.");

  }

  
  @ParameterizedTest
  @MethodSource("dataProvider")
  public void testGetItems(Item expectedItem) throws Exception {
    final TreasureChest chest = new TreasureChest();
    final List<Item> items = chest.getItems();
    assertNotNull(items);

    for (final Item item : items) {
      assertNotNull(item);
      assertNotNull(item.getType());
      assertNotNull(item.toString());

      final boolean sameType = expectedItem.getType() == item.getType();
      final boolean sameName = expectedItem.toString().equals(item.toString());
      if (sameType && sameName) {
        return;
      }
    }

    fail("Expected to find item [" + expectedItem + "] in the item list, but we didn't.");

  }

}