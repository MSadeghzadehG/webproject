
package com.iluwatar.stepbuilder;

import java.util.ArrayList;
import java.util.List;


public final class CharacterStepBuilder {

  private CharacterStepBuilder() {}

  public static NameStep newBuilder() {
    return new CharacterSteps();
  }

  
  public interface NameStep {
    ClassStep name(String name);
  }

  
  public interface ClassStep {
    WeaponStep fighterClass(String fighterClass);

    SpellStep wizardClass(String wizardClass);
  }

  
  public interface WeaponStep {
    AbilityStep withWeapon(String weapon);

    BuildStep noWeapon();
  }

  
  public interface SpellStep {
    AbilityStep withSpell(String spell);

    BuildStep noSpell();
  }

  
  public interface AbilityStep {
    AbilityStep withAbility(String ability);

    BuildStep noMoreAbilities();

    BuildStep noAbilities();
  }

  
  public interface BuildStep {
    Character build();
  }


  
  private static class CharacterSteps implements NameStep, ClassStep, WeaponStep, SpellStep,
      AbilityStep, BuildStep {

    private String name;
    private String fighterClass;
    private String wizardClass;
    private String weapon;
    private String spell;
    private List<String> abilities = new ArrayList<>();

    @Override
    public ClassStep name(String name) {
      this.name = name;
      return this;
    }

    @Override
    public WeaponStep fighterClass(String fighterClass) {
      this.fighterClass = fighterClass;
      return this;
    }

    @Override
    public SpellStep wizardClass(String wizardClass) {
      this.wizardClass = wizardClass;
      return this;
    }

    @Override
    public AbilityStep withWeapon(String weapon) {
      this.weapon = weapon;
      return this;
    }

    @Override
    public BuildStep noWeapon() {
      return this;
    }

    @Override
    public AbilityStep withSpell(String spell) {
      this.spell = spell;
      return this;
    }

    @Override
    public BuildStep noSpell() {
      return this;
    }

    @Override
    public AbilityStep withAbility(String ability) {
      this.abilities.add(ability);
      return this;
    }

    @Override
    public BuildStep noMoreAbilities() {
      return this;
    }

    @Override
    public BuildStep noAbilities() {
      return this;
    }

    @Override
    public Character build() {
      Character character = new Character(name);

      if (fighterClass != null) {
        character.setFighterClass(fighterClass);
      } else {
        character.setWizardClass(wizardClass);
      }

      if (weapon != null) {
        character.setWeapon(weapon);
      } else {
        character.setSpell(spell);
      }

      if (!abilities.isEmpty()) {
        character.setAbilities(abilities);
      }

      return character;
    }
  }
}
