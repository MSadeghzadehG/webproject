
package com.iluwatar.property;

import com.iluwatar.property.Character.Type;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;


public class App {

  private static final Logger LOGGER = LoggerFactory.getLogger(App.class);

  
  public static void main(String[] args) {
    
    Prototype charProto = new Character();
    charProto.set(Stats.STRENGTH, 10);
    charProto.set(Stats.AGILITY, 10);
    charProto.set(Stats.ARMOR, 10);
    charProto.set(Stats.ATTACK_POWER, 10);

    Character mageProto = new Character(Type.MAGE, charProto);
    mageProto.set(Stats.INTELLECT, 15);
    mageProto.set(Stats.SPIRIT, 10);

    Character warProto = new Character(Type.WARRIOR, charProto);
    warProto.set(Stats.RAGE, 15);
    warProto.set(Stats.ARMOR, 15); 
    Character rogueProto = new Character(Type.ROGUE, charProto);
    rogueProto.set(Stats.ENERGY, 15);
    rogueProto.set(Stats.AGILITY, 15); 
    
    Character mag = new Character("Player_1", mageProto);
    mag.set(Stats.ARMOR, 8);
    LOGGER.info(mag.toString());

    Character warrior = new Character("Player_2", warProto);
    LOGGER.info(warrior.toString());

    Character rogue = new Character("Player_3", rogueProto);
    LOGGER.info(rogue.toString());

    Character rogueDouble = new Character("Player_4", rogue);
    rogueDouble.set(Stats.ATTACK_POWER, 12);
    LOGGER.info(rogueDouble.toString());
  }
}
