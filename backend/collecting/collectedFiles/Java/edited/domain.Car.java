
package com.iluwatar.abstractdocument.domain;

import java.util.Map;

import com.iluwatar.abstractdocument.AbstractDocument;


public class Car extends AbstractDocument implements HasModel, HasPrice, HasParts {

  public Car(Map<String, Object> properties) {
    super(properties);
  }

}
