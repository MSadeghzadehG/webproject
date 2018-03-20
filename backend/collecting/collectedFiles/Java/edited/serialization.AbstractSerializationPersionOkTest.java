
package com.alibaba.dubbo.common.serialize.serialization;

import com.alibaba.dubbo.common.model.Person;

import org.junit.Test;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;

public abstract class AbstractSerializationPersionOkTest extends AbstractSerializationTest {
    @Test
    public void test_Person() throws Exception {
        assertObject(new Person());
    }

    @Test
    public void test_Person_withType() throws Exception {
        assertObjectWithType(new Person(), Person.class);
    }

    @Test
    public void test_PersonList() throws Exception {
        List<Person> args = new ArrayList<Person>();
        args.add(new Person());

        assertObject(args);
    }

    @Test
    public void test_PersonSet() throws Exception {
        Set<Person> args = new HashSet<Person>();
        args.add(new Person());

        assertObject(args);
    }

    @Test
    public void test_IntPersonMap() throws Exception {
        Map<Integer, Person> args = new HashMap<Integer, Person>();
        args.put(1, new Person());

        assertObject(args);
    }

    @Test
    public void test_StringPersonMap() throws Exception {
        Map<String, Person> args = new HashMap<String, Person>();
        args.put("1", new Person());

        assertObject(args);
    }

    @Test
    public void test_StringPersonListMap() throws Exception {
        Map<String, List<Person>> args = new HashMap<String, List<Person>>();

        List<Person> sublist = new ArrayList<Person>();
        sublist.add(new Person());
        args.put("1", sublist);

        assertObject(args);
    }

    @Test
    public void test_PersonListList() throws Exception {
        List<List<Person>> args = new ArrayList<List<Person>>();
        List<Person> sublist = new ArrayList<Person>();
        sublist.add(new Person());
        args.add(sublist);

        assertObject(args);
    }
}