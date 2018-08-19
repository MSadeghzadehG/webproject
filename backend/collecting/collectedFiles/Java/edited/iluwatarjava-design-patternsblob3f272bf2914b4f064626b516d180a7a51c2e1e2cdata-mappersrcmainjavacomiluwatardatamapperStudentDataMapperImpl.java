
package com.iluwatar.datamapper;

import java.util.ArrayList;
import java.util.List;
import java.util.Optional;


public final class StudentDataMapperImpl implements StudentDataMapper {

  
  private List<Student> students = new ArrayList<>();

  @Override
  public Optional<Student> find(int studentId) {

    
    for (final Student student : this.getStudents()) {

      
      if (student.getStudentId() == studentId) {

        return Optional.of(student);
      }
    }

    
    return Optional.empty();
  }

  @Override
  public void update(Student studentToBeUpdated) throws DataMapperException {


    
    if (this.getStudents().contains(studentToBeUpdated)) {

      
      final int index = this.getStudents().indexOf(studentToBeUpdated);

      
      this.getStudents().set(index, studentToBeUpdated);

    } else {

      
      throw new DataMapperException("Student [" + studentToBeUpdated.getName() + "] is not found");
    }
  }

  @Override
  public void insert(Student studentToBeInserted) throws DataMapperException {

    
    if (!this.getStudents().contains(studentToBeInserted)) {

      
      this.getStudents().add(studentToBeInserted);

    } else {

      
      throw new DataMapperException("Student already [" + studentToBeInserted.getName() + "] exists");
    }
  }

  @Override
  public void delete(Student studentToBeDeleted) throws DataMapperException {

    
    if (this.getStudents().contains(studentToBeDeleted)) {

      
      this.getStudents().remove(studentToBeDeleted);

    } else {

      
      throw new DataMapperException("Student [" + studentToBeDeleted.getName() + "] is not found");
    }
  }

  public List<Student> getStudents() {
    return this.students;
  }
}
