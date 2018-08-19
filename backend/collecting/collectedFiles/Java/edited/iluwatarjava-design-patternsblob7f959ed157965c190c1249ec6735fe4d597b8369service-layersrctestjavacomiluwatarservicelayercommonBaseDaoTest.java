
package com.iluwatar.servicelayer.common;

import com.iluwatar.servicelayer.hibernate.HibernateUtil;
import org.junit.jupiter.api.AfterEach;
import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.Test;

import java.util.List;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.function.Function;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertNotNull;
import static org.junit.jupiter.api.Assertions.assertNull;


public abstract class BaseDaoTest<E extends BaseEntity, D extends DaoBaseImpl<E>> {

  
  private static final int INITIAL_COUNT = 5;

  
  private static final AtomicInteger ID_GENERATOR = new AtomicInteger();

  
  private final Function<String, E> factory;

  
  private final D dao;

  
  public BaseDaoTest(final Function<String, E> factory, final D dao) {
    this.factory = factory;
    this.dao = dao;
  }

  @BeforeEach
  public void setUp() throws Exception {
    for (int i = 0; i < INITIAL_COUNT; i++) {
      final String className = dao.persistentClass.getSimpleName();
      final String entityName = String.format("%s%d", className, ID_GENERATOR.incrementAndGet());
      this.dao.persist(this.factory.apply(entityName));
    }
  }

  @AfterEach
  public void tearDown() throws Exception {
    HibernateUtil.dropSession();
  }

  protected final D getDao() {
    return this.dao;
  }

  @Test
  public void testFind() throws Exception {
    final List<E> all = this.dao.findAll();
    for (final E entity : all) {
      final E byId = this.dao.find(entity.getId());
      assertNotNull(byId);
      assertEquals(byId.getId(), byId.getId());
    }
  }

  @Test
  public void testDelete() throws Exception {
    final List<E> originalEntities = this.dao.findAll();
    this.dao.delete(originalEntities.get(1));
    this.dao.delete(originalEntities.get(2));

    final List<E> entitiesLeft = this.dao.findAll();
    assertNotNull(entitiesLeft);
    assertEquals(INITIAL_COUNT - 2, entitiesLeft.size());
  }

  @Test
  public void testFindAll() throws Exception {
    final List<E> all = this.dao.findAll();
    assertNotNull(all);
    assertEquals(INITIAL_COUNT, all.size());
  }

  @Test
  public void testSetId() throws Exception {
    final E entity = this.factory.apply("name");
    assertNull(entity.getId());

    final Long expectedId = Long.valueOf(1);
    entity.setId(expectedId);
    assertEquals(expectedId, entity.getId());
  }

  @Test
  public void testSetName() throws Exception {
    final E entity = this.factory.apply("name");
    assertEquals("name", entity.getName());
    assertEquals("name", entity.toString());

    final String expectedName = "new name";
    entity.setName(expectedName);
    assertEquals(expectedName, entity.getName());
    assertEquals(expectedName, entity.toString());
  }

}
