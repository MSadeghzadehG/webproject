
package com.iluwatar.repository;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertNull;
import static org.junit.jupiter.api.Assertions.assertTrue;

import java.util.Arrays;
import java.util.List;

import javax.annotation.Resource;

import org.junit.jupiter.api.AfterEach;
import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.extension.ExtendWith;
import org.springframework.test.context.ContextConfiguration;
import org.springframework.test.context.junit.jupiter.SpringExtension;

import com.google.common.collect.Lists;


@ExtendWith(SpringExtension.class)
@ContextConfiguration(locations = { "classpath:applicationContext.xml" })
public class RepositoryTest {

  @Resource
  private PersonRepository repository;

  Person peter = new Person("Peter", "Sagan", 17);
  Person nasta = new Person("Nasta", "Kuzminova", 25);
  Person john = new Person("John", "lawrence", 35);
  Person terry = new Person("Terry", "Law", 36);

  List<Person> persons = Arrays.asList(peter, nasta, john, terry);

  
  @BeforeEach
  public void setup() {

    repository.save(persons);
  }

  @Test
  public void testFindAll() {

    List<Person> actuals = Lists.newArrayList(repository.findAll());
    assertTrue(actuals.containsAll(persons) && persons.containsAll(actuals));
  }

  @Test
  public void testSave() {

    Person terry = repository.findByName("Terry");
    terry.setSurname("Lee");
    terry.setAge(47);
    repository.save(terry);

    terry = repository.findByName("Terry");
    assertEquals(terry.getSurname(), "Lee");
    assertEquals(47, terry.getAge());
  }

  @Test
  public void testDelete() {

    Person terry = repository.findByName("Terry");
    repository.delete(terry);

    assertEquals(3, repository.count());
    assertNull(repository.findByName("Terry"));
  }

  @Test
  public void testCount() {

    assertEquals(4, repository.count());
  }

  @Test
  public void testFindAllByAgeBetweenSpec() {

    List<Person> persons = repository.findAll(new PersonSpecifications.AgeBetweenSpec(20, 40));

    assertEquals(3, persons.size());
    assertTrue(persons.stream().allMatch((item) -> {
      return item.getAge() > 20 && item.getAge() < 40;
    }));
  }

  @Test
  public void testFindOneByNameEqualSpec() {

    Person actual = repository.findOne(new PersonSpecifications.NameEqualSpec("Terry"));
    assertEquals(terry, actual);
  }

  @AfterEach
  public void cleanup() {

    repository.deleteAll();
  }

}
