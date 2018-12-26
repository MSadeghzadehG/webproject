

package com.google.common.io;

import com.google.common.base.Charsets;
import com.google.common.testing.AbstractPackageSanityTests;
import java.lang.reflect.Method;
import java.nio.channels.FileChannel.MapMode;
import java.nio.charset.CharsetEncoder;



public class PackageSanityTests extends AbstractPackageSanityTests {
  public PackageSanityTests() {
    setDefault(BaseEncoding.class, BaseEncoding.base64());
    setDefault(int.class, 32);
    setDefault(String.class, "abcd");
    setDefault(Method.class, AbstractPackageSanityTests.class.getDeclaredMethods()[0]);
    setDefault(MapMode.class, MapMode.READ_ONLY);
    setDefault(CharsetEncoder.class, Charsets.UTF_8.newEncoder());
  }
}
