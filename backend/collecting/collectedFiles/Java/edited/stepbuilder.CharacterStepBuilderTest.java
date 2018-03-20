
package com.iluwatar.stepbuilder;

import org.junit.jupiter.api.Test;

import java.util.List;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertNotNull;
import static org.junit.jupiter.api.Assertions.assertNull;
import static org.junit.jupiter.api.Assertions.assertTrue;


public class CharacterStepBuilderTest {

  
  @Test
  public void testBuildWizard() {
    final Character character = CharacterStepBuilder.newBuilder()
        .name("Merlin")
        .wizardClass("alchemist")
        .withSpell("poison")
        .withAbility("invisibility")
        .withAbility("wisdom")
        .noMoreAbilities()
        .build();

    assertEquals("Merlin", character.getName());
    assertEquals("alchemist", character.getWizardClass());
    assertEquals("poison", character.getSpell());
    assertNotNull(character.toString());

    final List<String> abilities = character.getAbilities();
    assertNotNull(abilities);
    assertEquals(2, abilities.size());
    assertTrue(abilities.contains("invisibility"));
    assertTrue(abilities.contains("wisdom"));

  }

  
  @Test
  public void testBuildPoorWizard() {
    final Character character = CharacterStepBuilder.newBuilder()
        .name("Merlin")
        .wizardClass("alchemist")
        .noSpell()
        .build();

    assertEquals("Merlin", character.getName());
    assertEquals("alchemist", character.getWizardClass());
    assertNull(character.getSpell());
    assertNull(character.getAbilities());
    assertNotNull(character.toString());

  }

  
  @Test
  public void testBuildWeakWizard() {
    final Character character = CharacterStepBuilder.newBuilder()
        .name("Merlin")
        .wizardClass("alchemist")
        .withSpell("poison")
        .noAbilities()
        .build();

    assertEquals("Merlin", character.getName());
    assertEquals("alchemist", character.getWizardClass());
    assertEquals("poison", character.getSpell());
    assertNull(character.getAbilities());
    assertNotNull(character.toString());

  }


  
  @Test
  public void testBuildWarrior() {
    final Character character = CharacterStepBuilder.newBuilder()
        .name("Cuauhtemoc")
        .fighterClass("aztec")
        .withWeapon("spear")
        .withAbility("speed")
        .withAbility("strength")
        .noMoreAbilities()
        .build();

    assertEquals("Cuauhtemoc", character.getName());
    assertEquals("aztec", character.getFighterClass());
    assertEquals("spear", character.getWeapon());
    assertNotNull(character.toString());

    final List<String> abilities = character.getAbilities();
    assertNotNull(abilities);
    assertEquals(2, abilities.size());
    assertTrue(abilities.contains("speed"));
    assertTrue(abilities.contains("strength"));

  }

  
  @Test
  public void testBuildPoorWarrior() {
    final Character character = CharacterStepBuilder.newBuilder()
        .name("Poor warrior")
        .fighterClass("none")
        .noWeapon()
        .build();

    assertEquals("Poor warrior", character.getName());
    assertEquals("none", character.getFighterClass());
    assertNull(character.getWeapon());
    assertNull(character.getAbilities());
    assertNotNull(character.toString());

  }

  
  @Test
  public void testBuildWeakWarrior() {
    final Character character = CharacterStepBuilder.newBuilder()
        .name("Weak warrior")
        .fighterClass("none")
        .withWeapon("Slingshot")
        .noAbilities()
        .build();

    assertEquals("Weak warrior", character.getName());
    assertEquals("none", character.getFighterClass());
    assertEquals("Slingshot", character.getWeapon());
    assertNull(character.getAbilities());
    assertNotNull(character.toString());

  }

}