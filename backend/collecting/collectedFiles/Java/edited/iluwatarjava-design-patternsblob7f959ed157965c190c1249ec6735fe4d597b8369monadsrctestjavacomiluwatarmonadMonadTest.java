
package com.iluwatar.monad;

import org.junit.jupiter.api.Test;

import java.util.Objects;

import static org.junit.jupiter.api.Assertions.assertSame;
import static org.junit.jupiter.api.Assertions.assertThrows;


public class MonadTest {

  @Test
  public void testForInvalidName() {
    User tom = new User(null, 21, Sex.MALE, "tom@foo.bar");
    assertThrows(IllegalStateException.class, () -> {
      Validator.of(tom).validate(User::getName, Objects::nonNull, "name cannot be null").get();
    });
  }

  @Test
  public void testForInvalidAge() {
    User john = new User("John", 17, Sex.MALE, "john@qwe.bar");
    assertThrows(IllegalStateException.class, () -> {
      Validator.of(john).validate(User::getName, Objects::nonNull, "name cannot be null")
              .validate(User::getAge, age -> age > 21, "user is underaged")
              .get();
    });
  }

  @Test
  public void testForValid() {
    User sarah = new User("Sarah", 42, Sex.FEMALE, "sarah@det.org");
    User validated = Validator.of(sarah).validate(User::getName, Objects::nonNull, "name cannot be null")
        .validate(User::getAge, age -> age > 21, "user is underaged")
        .validate(User::getSex, sex -> sex == Sex.FEMALE, "user is not female")
        .validate(User::getEmail, email -> email.contains("@"), "email does not contain @ sign")
        .get();
    assertSame(validated, sarah);
  }
}
