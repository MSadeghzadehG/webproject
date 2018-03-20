
package com.iluwatar.datamapper;

import java.util.Optional;

import org.apache.log4j.Logger;


public final class App {

  private static Logger log = Logger.getLogger(App.class);

  
  public static void main(final String... args) {

    
    final StudentDataMapper mapper = new StudentDataMapperImpl();

    
    Student student = new Student(1, "Adam", 'A');

    
    mapper.insert(student);

    log.debug("App.main(), student : " + student + ", is inserted");

    
    final Optional<Student> studentToBeFound = mapper.find(student.getStudentId());

    log.debug("App.main(), student : " + studentToBeFound + ", is searched");

    
    student = new Student(student.getStudentId(), "AdamUpdated", 'A');

    
    mapper.update(student);

    log.debug("App.main(), student : " + student + ", is updated");
    log.debug("App.main(), student : " + student + ", is going to be deleted");

    
    mapper.delete(student);
  }

  private App() {}
}
