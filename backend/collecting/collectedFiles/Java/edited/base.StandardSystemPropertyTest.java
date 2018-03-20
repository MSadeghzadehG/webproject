

package com.google.common.base;

import static com.google.common.base.StandardSystemProperty.JAVA_COMPILER;
import static com.google.common.base.StandardSystemProperty.JAVA_EXT_DIRS;
import static com.google.common.truth.Truth.assertWithMessage;

import junit.framework.TestCase;


public class StandardSystemPropertyTest extends TestCase {

  public void testGetKeyMatchesString() {
    for (StandardSystemProperty property : StandardSystemProperty.values()) {
      String fieldName = property.name();
      String expected = Ascii.toLowerCase(fieldName).replaceAll("_", ".");
      assertEquals(expected, property.key());
    }
  }

  public void testGetValue() {
    for (StandardSystemProperty property : StandardSystemProperty.values()) {
      assertEquals(System.getProperty(property.key()), property.value());
    }
  }

  public void testToString() {
    for (StandardSystemProperty property : StandardSystemProperty.values()) {
      assertEquals(property.key() + "=" + property.value(), property.toString());
    }
  }

  public void testNoNullValues() {
    for (StandardSystemProperty property : StandardSystemProperty.values()) {
                  if (property == JAVA_COMPILER) {
        continue;
      }
                  if (property == JAVA_EXT_DIRS) {
        continue;
      }
      assertWithMessage(property.toString()).that(property.value()).isNotNull();
    }
  }
}
