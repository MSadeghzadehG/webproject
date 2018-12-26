
package com.iluwatar.pageobject.pages;

import com.gargoylesoftware.htmlunit.WebClient;
import com.gargoylesoftware.htmlunit.html.HtmlAnchor;
import com.gargoylesoftware.htmlunit.html.HtmlPage;

import java.io.IOException;
import java.util.List;


public class AlbumListPage extends Page {

  private static final String ALBUM_LIST_HTML_FILE = "album-list.html";
  private static final String PAGE_URL = "file:" + AUT_PATH + ALBUM_LIST_HTML_FILE;

  private HtmlPage page;


  
  public AlbumListPage(WebClient webClient) {
    super(webClient);
  }


  
  public AlbumListPage navigateToPage() {
    try {
      page = this.webClient.getPage(PAGE_URL);
    } catch (IOException e) {
      e.printStackTrace();
    }
    return this;
  }

  
  @Override
  public boolean isAt() {
    return "Album List".equals(page.getTitleText());
  }

  
  public AlbumPage selectAlbum(String albumTitle) {
        List<HtmlAnchor> albumLinks = (List<HtmlAnchor>) page.getByXPath("    for (HtmlAnchor anchor : albumLinks) {
      if (anchor.getTextContent().equals(albumTitle)) {
        try {
          anchor.click();
          return new AlbumPage(webClient);
        } catch (IOException e) {
          e.printStackTrace();
        }
      }
    }
    throw new IllegalArgumentException("No links with the album title: " + albumTitle);
  }


}
