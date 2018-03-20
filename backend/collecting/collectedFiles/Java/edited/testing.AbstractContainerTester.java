

package com.google.common.collect.testing;

import com.google.common.annotations.GwtCompatible;
import com.google.errorprone.annotations.OverridingMethodsMustInvokeSuper;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;
import java.util.List;
import org.junit.Ignore;


@GwtCompatible
@Ignore public abstract class AbstractContainerTester<C, E>
    extends AbstractTester<OneSizeTestContainerGenerator<C, E>> {
  protected SampleElements<E> samples;
  protected C container;

  @Override
  @OverridingMethodsMustInvokeSuper
  public void setUp() throws Exception {
    super.setUp();
    samples = this.getSubjectGenerator().samples();
    resetContainer();
  }

  
  protected abstract Collection<E> actualContents();

  
  protected C resetContainer() {
    return resetContainer(getSubjectGenerator().createTestSubject());
  }

  
  protected C resetContainer(C newValue) {
    container = newValue;
    return container;
  }

  
  protected final void expectContents(E... elements) {
    expectContents(Arrays.asList(elements));
  }

  
  
  protected void expectContents(Collection<E> expected) {
    Helpers.assertEqualIgnoringOrder(expected, actualContents());
  }

  protected void expectUnchanged() {
    expectContents(getOrderedElements());
  }

  
  protected final void expectAdded(E... elements) {
    List<E> expected = Helpers.copyToList(getSampleElements());
    expected.addAll(Arrays.asList(elements));
    expectContents(expected);
  }

  protected final void expectAdded(int index, E... elements) {
    expectAdded(index, Arrays.asList(elements));
  }

  protected final void expectAdded(int index, Collection<E> elements) {
    List<E> expected = Helpers.copyToList(getSampleElements());
    expected.addAll(index, elements);
    expectContents(expected);
  }

  
  protected void expectMissing(E... elements) {
    for (E element : elements) {
      assertFalse("Should not contain " + element, actualContents().contains(element));
    }
  }

  protected E[] createSamplesArray() {
    E[] array = getSubjectGenerator().createArray(getNumElements());
    getSampleElements().toArray(array);
    return array;
  }

  protected E[] createOrderedArray() {
    E[] array = getSubjectGenerator().createArray(getNumElements());
    getOrderedElements().toArray(array);
    return array;
  }

  public static class ArrayWithDuplicate<E> {
    public final E[] elements;
    public final E duplicate;

    private ArrayWithDuplicate(E[] elements, E duplicate) {
      this.elements = elements;
      this.duplicate = duplicate;
    }
  }

  
  protected ArrayWithDuplicate<E> createArrayWithDuplicateElement() {
    E[] elements = createSamplesArray();
    E duplicate = elements[(elements.length / 2) - 1];
    elements[(elements.length / 2) + 1] = duplicate;
    return new ArrayWithDuplicate<E>(elements, duplicate);
  }

  
  protected int getNumElements() {
    return getSubjectGenerator().getCollectionSize().getNumElements();
  }

  protected Collection<E> getSampleElements(int howMany) {
    return getSubjectGenerator().getSampleElements(howMany);
  }

  protected Collection<E> getSampleElements() {
    return getSampleElements(getNumElements());
  }

  
  protected List<E> getOrderedElements() {
    List<E> list = new ArrayList<E>();
    for (E e : getSubjectGenerator().order(new ArrayList<E>(getSampleElements()))) {
      list.add(e);
    }
    return Collections.unmodifiableList(list);
  }

  
  protected int getNullLocation() {
    return getNumElements() / 2;
  }

  @SuppressWarnings("unchecked")
  protected MinimalCollection<E> createDisjointCollection() {
    return MinimalCollection.of(e3(), e4());
  }

  @SuppressWarnings("unchecked")
  protected MinimalCollection<E> emptyCollection() {
    return MinimalCollection.<E>of();
  }

  protected final E e0() {
    return samples.e0();
  }

  protected final E e1() {
    return samples.e1();
  }

  protected final E e2() {
    return samples.e2();
  }

  protected final E e3() {
    return samples.e3();
  }

  protected final E e4() {
    return samples.e4();
  }
}
