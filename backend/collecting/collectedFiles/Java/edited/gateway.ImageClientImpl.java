
package com.iluwatar.api.gateway;

import org.apache.http.client.methods.CloseableHttpResponse;
import org.apache.http.client.methods.HttpGet;
import org.apache.http.impl.client.CloseableHttpClient;
import org.apache.http.impl.client.HttpClients;
import org.apache.http.util.EntityUtils;
import org.springframework.stereotype.Component;

import java.io.IOException;


@Component
public class ImageClientImpl implements ImageClient {
  
  @Override
  public String getImagePath() {
    String response = null;
    try (CloseableHttpClient httpClient = HttpClients.createDefault()) {
      HttpGet httpGet = new HttpGet("http:      try (CloseableHttpResponse httpResponse = httpClient.execute(httpGet)) {
        response = EntityUtils.toString(httpResponse.getEntity());
      }
    } catch (IOException e) {
      e.printStackTrace();
    }
    return response;
  }
}
