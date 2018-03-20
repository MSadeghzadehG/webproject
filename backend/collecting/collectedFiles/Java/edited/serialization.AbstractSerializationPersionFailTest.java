
package com.alibaba.dubbo.common.serialize.serialization;

import com.alibaba.dubbo.common.model.Person;
import com.alibaba.dubbo.common.serialize.ObjectOutput;

import org.junit.Test;

import java.io.NotSerializableException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;

import static org.junit.Assert.assertThat;
import static org.junit.Assert.fail;
import static org.junit.matchers.JUnitMatchers.containsString;

public abstract class AbstractSerializationPersionFailTest extends AbstractSerializationTest {
    @Test
    public void test_Person() throws Exception {
        try {
            ObjectOutput objectOutput = serialization.serialize(url, byteArrayOutputStream);
            objectOutput.writeObject(new Person());
            fail();
        } catch (NotSerializableException expected) {
        } catch (IllegalStateException expected) {
            assertThat(expected.getMessage(), containsString("Serialized class com.alibaba.dubbo.common.model.Person must implement java.io.Serializable"));
        }
    }

    @Test
    public void test_PersonList() throws Exception {
        List<Person> args = new ArrayList<Person>();
        args.add(new Person());
        try {
            ObjectOutput objectOutput = serialization.serialize(url, byteArrayOutputStream);
            objectOutput.writeObject(args);
            fail();
        } catch (NotSerializableException expected) {
        } catch (IllegalStateException expected) {
            assertThat(expected.getMessage(), containsString("Serialized class com.alibaba.dubbo.common.model.Person must implement java.io.Serializable"));
        }
    }

    @Test
    public void test_PersonSet() throws Exception {
        Set<Person> args = new HashSet<Person>();
        args.add(new Person());
        try {
            ObjectOutput objectOutput = serialization.serialize(url, byteArrayOutputStream);
            objectOutput.writeObject(args);
            fail();
        } catch (NotSerializableException expected) {
        } catch (IllegalStateException expected) {
            assertThat(expected.getMessage(), containsString("Serialized class com.alibaba.dubbo.common.model.Person must implement java.io.Serializable"));
        }
    }

    @Test
    public void test_IntPersonMap() throws Exception {
        Map<Integer, Person> args = new HashMap<Integer, Person>();
        args.put(1, new Person());
        try {
            ObjectOutput objectOutput = serialization.serialize(url, byteArrayOutputStream);
            objectOutput.writeObject(args);
            fail();
        } catch (NotSerializableException expected) {
        } catch (IllegalStateException expected) {
            assertThat(expected.getMessage(), containsString("Serialized class com.alibaba.dubbo.common.model.Person must implement java.io.Serializable"));
        }
    }

    @Test
    public void test_StringPersonMap() throws Exception {
        Map<String, Person> args = new HashMap<String, Person>();
        args.put("1", new Person());
        try {
            ObjectOutput objectOutput = serialization.serialize(url, byteArrayOutputStream);
            objectOutput.writeObject(args);
            fail();
        } catch (NotSerializableException expected) {
        } catch (IllegalStateException expected) {
            assertThat(expected.getMessage(), containsString("Serialized class com.alibaba.dubbo.common.model.Person must implement java.io.Serializable"));
        }
    }

    @Test
    public void test_StringPersonListMap() throws Exception {
        Map<String, List<Person>> args = new HashMap<String, List<Person>>();

        List<Person> sublist = new ArrayList<Person>();
        sublist.add(new Person());
        args.put("1", sublist);
        try {
            ObjectOutput objectOutput = serialization.serialize(url, byteArrayOutputStream);
            objectOutput.writeObject(args);
            fail();
        } catch (NotSerializableException expected) {
        } catch (IllegalStateException expected) {
            assertThat(expected.getMessage(), containsString("Serialized class com.alibaba.dubbo.common.model.Person must implement java.io.Serializable"));
        }
    }

    @Test
    public void test_PersonListList() throws Exception {
        List<List<Person>> args = new ArrayList<List<Person>>();
        List<Person> sublist = new ArrayList<Person>();
        sublist.add(new Person());
        args.add(sublist);
        try {
            ObjectOutput objectOutput = serialization.serialize(url, byteArrayOutputStream);
            objectOutput.writeObject(args);
            fail();
        } catch (NotSerializableException expected) {
        } catch (IllegalStateException expected) {
            assertThat(expected.getMessage(), containsString("Serialized class com.alibaba.dubbo.common.model.Person must implement java.io.Serializable"));
        }
    }
}