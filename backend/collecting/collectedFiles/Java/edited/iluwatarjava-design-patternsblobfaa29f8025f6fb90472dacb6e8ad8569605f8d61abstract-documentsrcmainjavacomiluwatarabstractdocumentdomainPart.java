
package com.iluwatar.abstractdocument.domain;

import java.util.Map;

import com.iluwatar.abstractdocument.AbstractDocument;


public class Part extends AbstractDocument implements HasType, HasModel, HasPrice {

  public Part(Map<String, Object> properties) {
    super(properties);
  }

}
