
package com.iluwatar.repository;

import java.sql.SQLException;
import java.util.List;
import java.util.Properties;

import javax.sql.DataSource;

import org.apache.commons.dbcp.BasicDataSource;
import org.hibernate.jpa.HibernatePersistenceProvider;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.context.annotation.AnnotationConfigApplicationContext;
import org.springframework.context.annotation.Bean;
import org.springframework.data.jpa.repository.config.EnableJpaRepositories;
import org.springframework.orm.jpa.JpaTransactionManager;
import org.springframework.orm.jpa.LocalContainerEntityManagerFactoryBean;


@EnableJpaRepositories
public class AppConfig {

  private static final Logger LOGGER = LoggerFactory.getLogger(AppConfig.class);

  
  @Bean(destroyMethod = "close")
  public DataSource dataSource() {
    BasicDataSource basicDataSource = new BasicDataSource();
    basicDataSource.setDriverClassName("org.h2.Driver");
    basicDataSource.setUrl("jdbc:h2:~/databases/person");
    basicDataSource.setUsername("sa");
    basicDataSource.setPassword("sa");
    return (DataSource) basicDataSource;
  }

  
  @Bean
  public LocalContainerEntityManagerFactoryBean entityManagerFactory() {
    LocalContainerEntityManagerFactoryBean entityManager = new LocalContainerEntityManagerFactoryBean();
    entityManager.setDataSource(dataSource());
    entityManager.setPackagesToScan("com.iluwatar");
    entityManager.setPersistenceProvider(new HibernatePersistenceProvider());
    entityManager.setJpaProperties(jpaProperties());

    return entityManager;
  }

  
  private static Properties jpaProperties() {
    Properties properties = new Properties();
    properties.setProperty("hibernate.dialect", "org.hibernate.dialect.H2Dialect");
    properties.setProperty("hibernate.hbm2ddl.auto", "create-drop");
    return properties;
  }

  
  @Bean
  public JpaTransactionManager transactionManager() throws SQLException {
    JpaTransactionManager transactionManager = new JpaTransactionManager();
    transactionManager.setEntityManagerFactory(entityManagerFactory().getObject());
    return transactionManager;
  }

  
  public static void main(String[] args) {

    AnnotationConfigApplicationContext context = new AnnotationConfigApplicationContext(
        AppConfig.class);
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

    context.close();

  }

}
