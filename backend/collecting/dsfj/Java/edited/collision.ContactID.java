

package org.jbox2d.collision;


public class ContactID implements Comparable<ContactID> {

  public static enum Type {
    VERTEX, FACE
  }

  public byte indexA;
  public byte indexB;
  public byte typeA;
  public byte typeB;

  public int getKey() {
    return ((int) indexA) << 24 | ((int) indexB) << 16 | ((int) typeA) << 8 | ((int) typeB);
  }

  public boolean isEqual(final ContactID cid) {
    return getKey() == cid.getKey();
  }

  public ContactID() {}

  public ContactID(final ContactID c) {
    set(c);
  }

  public void set(final ContactID c) {
    indexA = c.indexA;
    indexB = c.indexB;
    typeA = c.typeA;
    typeB = c.typeB;
  }

  public void flip() {
    byte tempA = indexA;
    indexA = indexB;
    indexB = tempA;
    tempA = typeA;
    typeA = typeB;
    typeB = tempA;
  }

  
  public void zero() {
    indexA = 0;
    indexB = 0;
    typeA = 0;
    typeB = 0;
  }

  @Override
  public int compareTo(ContactID o) {
    return getKey() - o.getKey();
  }
}
