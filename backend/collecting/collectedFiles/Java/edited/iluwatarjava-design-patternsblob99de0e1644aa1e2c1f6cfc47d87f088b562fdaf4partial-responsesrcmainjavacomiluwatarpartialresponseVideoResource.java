

package com.iluwatar.partialresponse;

import java.util.Map;


public class VideoResource {
  private FieldJsonMapper fieldJsonMapper;
  private Map<Integer, Video> videos;

  
  public VideoResource(FieldJsonMapper fieldJsonMapper, Map<Integer, Video> videos) {
    this.fieldJsonMapper = fieldJsonMapper;
    this.videos = videos;
  }

  
  public String getDetails(Integer id, String... fields) throws Exception {
    if (fields.length == 0) {
      return videos.get(id).toString();
    }
    return fieldJsonMapper.toJson(videos.get(id), fields);
  }
}
