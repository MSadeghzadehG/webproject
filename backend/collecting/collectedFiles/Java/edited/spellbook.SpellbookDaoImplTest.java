
package com.iluwatar.servicelayer.spellbook;

import com.iluwatar.servicelayer.common.BaseDaoTest;
import org.junit.jupiter.api.Test;

import java.util.List;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertNotNull;


public class SpellbookDaoImplTest extends BaseDaoTest<Spellbook, SpellbookDaoImpl> {

  public SpellbookDaoImplTest() {
    super(Spellbook::new, new SpellbookDaoImpl());
  }

  @Test
  public void testFindByName() throws Exception {
    final SpellbookDaoImpl dao = getDao();
    final List<Spellbook> allBooks = dao.findAll();
    for (final Spellbook book : allBooks) {
      final Spellbook spellByName = dao.findByName(book.getName());
      assertNotNull(spellByName);
      assertEquals(book.getId(), spellByName.getId());
      assertEquals(book.getName(), spellByName.getName());
    }
  }

}
