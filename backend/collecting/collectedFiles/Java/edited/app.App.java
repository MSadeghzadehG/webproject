
package com.iluwatar.fluentinterface.app;

import static java.lang.String.valueOf;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Iterator;
import java.util.List;
import java.util.StringJoiner;
import java.util.function.Function;
import java.util.function.Predicate;

import com.iluwatar.fluentinterface.fluentiterable.FluentIterable;
import com.iluwatar.fluentinterface.fluentiterable.lazy.LazyFluentIterable;
import com.iluwatar.fluentinterface.fluentiterable.simple.SimpleFluentIterable;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;


public class App {

  private static final Logger LOGGER = LoggerFactory.getLogger(App.class);

  
  public static void main(String[] args) {

    List<Integer> integerList = new ArrayList<>();
    integerList.addAll(Arrays.asList(1, -61, 14, -22, 18, -87, 6, 64, -82, 26, -98, 97, 45, 23, 2,
        -68, 45));

    prettyPrint("The initial list contains: ", integerList);

    List<Integer> firstFiveNegatives =
        SimpleFluentIterable.fromCopyOf(integerList).filter(negatives()).first(3).asList();
    prettyPrint("The first three negative values are: ", firstFiveNegatives);


    List<Integer> lastTwoPositives =
        SimpleFluentIterable.fromCopyOf(integerList).filter(positives()).last(2).asList();
    prettyPrint("The last two positive values are: ", lastTwoPositives);

    SimpleFluentIterable
        .fromCopyOf(integerList)
        .filter(number -> number % 2 == 0)
        .first()
        .ifPresent(evenNumber -> LOGGER.info("The first even number is: {}", evenNumber));


    List<String> transformedList =
        SimpleFluentIterable.fromCopyOf(integerList).filter(negatives()).map(transformToString())
            .asList();
    prettyPrint("A string-mapped list of negative numbers contains: ", transformedList);


    List<String> lastTwoOfFirstFourStringMapped =
        LazyFluentIterable.from(integerList).filter(positives()).first(4).last(2)
            .map(number -> "String[" + valueOf(number) + "]").asList();
    prettyPrint(
        "The lazy list contains the last two of the first four positive numbers mapped to Strings: ",
        lastTwoOfFirstFourStringMapped);

    LazyFluentIterable
        .from(integerList)
        .filter(negatives())
        .first(2)
        .last()
        .ifPresent(lastOfFirstTwo -> LOGGER.info("The last of the first two negatives is: {}", lastOfFirstTwo));
  }

  private static Function<Integer, String> transformToString() {
    return integer -> "String[" + valueOf(integer) + "]";
  }

  private static Predicate<? super Integer> negatives() {
    return integer -> integer < 0;
  }

  private static Predicate<? super Integer> positives() {
    return integer -> integer > 0;
  }

  private static <E> void prettyPrint(String prefix, Iterable<E> iterable) {
    prettyPrint(", ", prefix, iterable);
  }

  private static <E> void prettyPrint(String delimiter, String prefix,
                                         Iterable<E> iterable) {
    StringJoiner joiner = new StringJoiner(delimiter, prefix, ".");
    Iterator<E> iterator = iterable.iterator();
    while (iterator.hasNext()) {
      joiner.add(iterator.next().toString());
    }

    LOGGER.info(joiner.toString());
  }
}
