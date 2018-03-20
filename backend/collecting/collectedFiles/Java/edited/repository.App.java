
package com.iluwatar.repository;

import java.util.List;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.context.support.ClassPathXmlApplicationContext;


public class App {

  private static final Logger LOGGER = LoggerFactory.getLogger(App.class);

  
  public static void main(String[] args) {

    ClassPathXmlApplicationContext context = new ClassPathXmlApplicationContext(
        "applicationContext.xml");
    PersonRepository repository = context.getBean(PersonRepository.class);

    Person peter = new Person("Peter", "Sagan", 17);
    Person nasta = new Person("Nasta", "Kuzminova", 25);
    Person john = new Person("John", "lawrence", 35);
    Person terry = new Person("Terry", "Law", 36);

        repository.save(peter);
    repository.save(nasta);
    repository.save(john);
    repository.save(terry);

        LOGGER.info("Count Person records: {}", repository.count());

        List<Person> persons = (List<Person>) repository.findAll();
    for (Person person : persons) {
      LOGGER.info(person.toString());
    }

        nasta.setName("Barbora");
    nasta.setSurname("Spotakova");
    repository.save(nasta);

    LOGGER.info("Find by id 2: {}", repository.findOne(2L));

        repository.delete(2L);

        LOGGER.info("Count Person records: {}", repository.count());

        Person p = repository.findOne(new PersonSpecifications.NameEqualSpec("John"));
    LOGGER.info("Find by John is {}", p);

        persons = repository.findAll(new PersonSpecifications.AgeBetweenSpec(20, 40));

    LOGGER.info("Find Person with age between 20,40: ");
    for (Person person : persons) {
      LOGGER.info(person.toString());
    }

    repository.deleteAll();
    
    context.close();

  }
}
