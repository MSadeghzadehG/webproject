
package com.iluwatar.builder;

import com.iluwatar.builder.Hero.Builder;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;


public class App {

  private static final Logger LOGGER = LoggerFactory.getLogger(App.class);

  
  public static void main(String[] args) {

    Hero mage =
        new Hero.Builder(Profession.MAGE, "Riobard").withHairColor(HairColor.BLACK)
            .withWeapon(Weapon.DAGGER).build();
    LOGGER.info(mage.toString());

    Hero warrior =
        new Hero.Builder(Profession.WARRIOR, "Amberjill").withHairColor(HairColor.BLOND)
            .withHairType(HairType.LONG_CURLY).withArmor(Armor.CHAIN_MAIL).withWeapon(Weapon.SWORD)
            .build();
    LOGGER.info(warrior.toString());

    Hero thief =
        new Hero.Builder(Profession.THIEF, "Desmond").withHairType(HairType.BALD)
            .withWeapon(Weapon.BOW).build();
    LOGGER.info(thief.toString());

  }
}
