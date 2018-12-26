

package com.google.common.io;

import com.google.common.collect.ImmutableSet;
import java.io.File;
import java.io.IOException;
import junit.framework.TestCase;



public class FileTreeTraverserTest extends TestCase {

  private File dir;

  @Override
  public void setUp() throws IOException {
    dir = Files.createTempDir();
  }

  @Override
  public void tearDown() throws IOException {
    File[] files = dir.listFiles();
    if (files == null) {
      return;
    }

        for (File file : files) {
      file.delete();
    }

    dir.delete();
  }

  public void testFileTreeViewer_emptyDir() throws IOException {
    assertDirChildren();
  }

  public void testFileTreeViewer_singleFile() throws IOException {
    File file = newFile("test");
    assertDirChildren(file);
  }

  public void testFileTreeViewer_singleDir() throws IOException {
    File file = newDir("test");
    assertDirChildren(file);
  }

  public void testFileTreeViewer_multipleFiles() throws IOException {
    File a = newFile("a");
    File b = newDir("b");
    File c = newFile("c");
    File d = newDir("d");
    assertDirChildren(a, b, c, d);
  }

  private File newDir(String name) throws IOException {
    File file = new File(dir, name);
    file.mkdir();
    return file;
  }

  private File newFile(String name) throws IOException {
    File file = new File(dir, name);
    file.createNewFile();
    return file;
  }

  private void assertDirChildren(File... files) {
    assertEquals(
        ImmutableSet.copyOf(files), ImmutableSet.copyOf(Files.fileTreeTraverser().children(dir)));
  }
}
