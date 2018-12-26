
package com.iluwatar.abstractdocument.domain;

import java.util.Optional;

import com.iluwatar.abstractdocument.Document;


public interface HasPrice extends Document {

  String PROPERTY = "price";

  default Optional<Number> getPrice() {
    return Optional.ofNullable((Number) get(PROPERTY));
  }

}
