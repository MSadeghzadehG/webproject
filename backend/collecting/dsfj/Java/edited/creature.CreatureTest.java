
package com.iluwatar.specification.creature;

import com.iluwatar.specification.property.Color;
import com.iluwatar.specification.property.Movement;
import com.iluwatar.specification.property.Size;
import org.junit.jupiter.params.ParameterizedTest;
import org.junit.jupiter.params.provider.MethodSource;

import java.util.Arrays;
import java.util.Collection;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertNotNull;


public class CreatureTest {

  
  public static Collection<Object[]> dataProvider() {
    return Arrays.asList(
            new Object[]{new Dragon(), "Dragon", Size.LARGE, Movement.FLYING, Color.RED},
            new Object[]{new Goblin(), "Goblin", Size.SMALL, Movement.WALKING, Color.GREEN},
            new Object[]{new KillerBee(), "KillerBee", Size.SMALL, Movement.FLYING, Color.LIGHT},
            new Object[]{new Octopus(), "Octopus", Size.NORMAL, Movement.SWIMMING, Color.DARK},
            new Object[]{new Shark(), "Shark", Size.NORMAL, Movement.SWIMMING, Color.LIGHT},
            new Object[]{new Troll(), "Troll", Size.LARGE, Movement.WALKING, Color.DARK}
    );
  }

  @ParameterizedTest
  @MethodSource("dataProvider")
  public void testGetName(Creature testedCreature, String name) throws Exception {
    assertEquals(name, testedCreature.getName());
  }

  @ParameterizedTest
  @MethodSource("dataProvider")
  public void testGetSize(Creature testedCreature, String name, Size size) throws Exception {
    assertEquals(size, testedCreature.getSize());
  }

  @ParameterizedTest
  @MethodSource("dataProvider")
  public void testGetMovement(Creature testedCreature, String name, Size size, Movement movement) throws Exception {
    assertEquals(movement, testedCreature.getMovement());
  }

  @ParameterizedTest
  @MethodSource("dataProvider")
  public void testGetColor(Creature testedCreature, String name, Size size, Movement movement,
                           Color color) throws Exception {
    assertEquals(color, testedCreature.getColor());
  }

  @ParameterizedTest
  @MethodSource("dataProvider")
  public void testToString(Creature testedCreature, String name, Size size, Movement movement,
                           Color color) throws Exception {
    final String toString = testedCreature.toString();
    assertNotNull(toString);
    assertEquals(
            String.format("%s [size=%s, movement=%s, color=%s]", name, size, movement, color),
            toString
    );
  }
}