

package com.iluwatar.dao;

import java.util.Optional;
import java.util.stream.Stream;


public interface CustomerDao {

  
  Stream<Customer> getAll() throws Exception;
  
  
  Optional<Customer> getById(int id) throws Exception;

  
  boolean add(Customer customer) throws Exception;

  
  boolean update(Customer customer) throws Exception;

  
  boolean delete(Customer customer) throws Exception;
}
