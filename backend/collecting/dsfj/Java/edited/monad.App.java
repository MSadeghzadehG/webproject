
package com.iluwatar.monad;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.util.Objects;
import java.util.function.Function;
import java.util.function.Predicate;


public class App {

  private static final Logger LOGGER = LoggerFactory.getLogger(App.class);

  
  public static void main(String[] args) {
    User user = new User("user", 24, Sex.FEMALE, "foobar.com");
    LOGGER.info(Validator.of(user).validate(User::getName, Objects::nonNull, "name is null")
        .validate(User::getName, name -> !name.isEmpty(), "name is empty")
        .validate(User::getEmail, email -> !email.contains("@"), "email doesn't containt '@'")
        .validate(User::getAge, age -> age > 20 && age < 30, "age isn't between...").get().toString());
  }
}
