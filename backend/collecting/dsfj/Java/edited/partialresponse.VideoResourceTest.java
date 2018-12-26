

package com.iluwatar.partialresponse;

import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mock;
import org.mockito.runners.MockitoJUnitRunner;

import java.util.HashMap;
import java.util.Map;

import static org.junit.Assert.assertEquals;
import static org.mockito.Matchers.any;
import static org.mockito.Matchers.eq;
import static org.mockito.Mockito.when;


@RunWith(MockitoJUnitRunner.class)
public class VideoResourceTest {
  @Mock
  private FieldJsonMapper fieldJsonMapper;

  private VideoResource resource;

  @Before
  public void setUp() {
    Map<Integer, Video> videos = new HashMap<>();
    videos.put(1, new Video(1, "Avatar", 178, "epic science fiction film", "James Cameron", "English"));
    videos.put(2, new Video(2, "Godzilla Resurgence", 120, "Action & drama movie|", "Hideaki Anno", "Japanese"));
    videos.put(3, new Video(3, "Interstellar", 169, "Adventure & Sci-Fi", "Christopher Nolan", "English"));
    resource = new VideoResource(fieldJsonMapper, videos);
  }

  @Test
  public void shouldGiveVideoDetailsById() throws Exception {
    String actualDetails = resource.getDetails(1);

    String expectedDetails = "{\"id\": 1,\"title\": \"Avatar\",\"length\": 178,\"description\": "
        + "\"epic science fiction film\",\"director\": \"James Cameron\",\"language\": \"English\",}";
    assertEquals(expectedDetails, actualDetails);
  }

  @Test
  public void shouldGiveSpecifiedFieldsInformationOfVideo() throws Exception {
    String[] fields = new String[]{"id", "title", "length"};

    String expectedDetails = "{\"id\": 1,\"title\": \"Avatar\",\"length\": 178}";
    when(fieldJsonMapper.toJson(any(Video.class), eq(fields))).thenReturn(expectedDetails);

    String actualFieldsDetails = resource.getDetails(2, fields);

    assertEquals(expectedDetails, actualFieldsDetails);
  }
}