
package com.iluwatar.servicelayer.spell;

import com.iluwatar.servicelayer.common.BaseDaoTest;
import org.junit.jupiter.api.Test;

import java.util.List;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertNotNull;


public class SpellDaoImplTest extends BaseDaoTest<Spell, SpellDaoImpl> {

  public SpellDaoImplTest() {
    super(Spell::new, new SpellDaoImpl());
  }

  @Test
  public void testFindByName() throws Exception {
    final SpellDaoImpl dao = getDao();
    final List<Spell> allSpells = dao.findAll();
    for (final Spell spell : allSpells) {
      final Spell spellByName = dao.findByName(spell.getName());
      assertNotNull(spellByName);
      assertEquals(spell.getId(), spellByName.getId());
      assertEquals(spell.getName(), spellByName.getName());
    }
  }

}
