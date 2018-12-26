
package com.fernandocejas.android10.sample.data.cache;

import com.fernandocejas.android10.sample.data.ApplicationTestCase;
import java.io.File;
import org.junit.After;
import org.junit.Before;
import org.junit.Test;

import static org.hamcrest.CoreMatchers.equalTo;
import static org.hamcrest.CoreMatchers.is;
import static org.hamcrest.MatcherAssert.assertThat;

public class FileManagerTest extends ApplicationTestCase {

  private FileManager fileManager;

  @Before
  public void setUp() {
    fileManager = new FileManager();
  }

  @After
  public void tearDown() {
    if (cacheDir() != null) {
      fileManager.clearDirectory(cacheDir());
    }
  }

  @Test
  public void testWriteToFile() {
    File fileToWrite = createDummyFile();
    String fileContent = "content";

    fileManager.writeToFile(fileToWrite, fileContent);

    assertThat(fileToWrite.exists(), is(true));
  }

  @Test
  public void testFileContent() {
    File fileToWrite = createDummyFile();
    String fileContent = "content\n";

    fileManager.writeToFile(fileToWrite, fileContent);
    String expectedContent = fileManager.readFileContent(fileToWrite);

    assertThat(expectedContent, is(equalTo(fileContent)));
  }

  private File createDummyFile() {
    String dummyFilePath = cacheDir().getPath() + File.separator + "dummyFile";
    return new File(dummyFilePath);
  }
}
