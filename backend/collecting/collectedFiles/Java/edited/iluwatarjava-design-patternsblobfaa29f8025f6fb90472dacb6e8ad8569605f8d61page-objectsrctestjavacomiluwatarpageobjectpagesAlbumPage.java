
package com.iluwatar.pageobject.pages;

import com.gargoylesoftware.htmlunit.WebClient;
import com.gargoylesoftware.htmlunit.html.HtmlNumberInput;
import com.gargoylesoftware.htmlunit.html.HtmlOption;
import com.gargoylesoftware.htmlunit.html.HtmlPage;
import com.gargoylesoftware.htmlunit.html.HtmlSelect;
import com.gargoylesoftware.htmlunit.html.HtmlSubmitInput;
import com.gargoylesoftware.htmlunit.html.HtmlTextInput;

import java.io.IOException;


public class AlbumPage extends Page {

  private static final String ALBUM_PAGE_HTML_FILE = "album-page.html";
  private static final String PAGE_URL = "file:" + AUT_PATH + ALBUM_PAGE_HTML_FILE;

  private HtmlPage page;


  
  public AlbumPage(WebClient webClient) {
    super(webClient);
  }


  
  public AlbumPage navigateToPage() {
    try {
      page = this.webClient.getPage(PAGE_URL);
    } catch (IOException e) {
      e.printStackTrace();
    }
    return this;
  }


  
  @Override
  public boolean isAt() {
    return "Album Page".equals(page.getTitleText());
  }


  
  public AlbumPage changeAlbumTitle(String albumTitle) {
    HtmlTextInput albumTitleInputTextField = (HtmlTextInput) page.getElementById("albumTitle");
    albumTitleInputTextField.setText(albumTitle);
    return this;
  }


  
  public AlbumPage changeArtist(String artist) {
    HtmlTextInput artistInputTextField = (HtmlTextInput) page.getElementById("albumArtist");
    artistInputTextField.setText(artist);
    return this;
  }


  
  public AlbumPage changeAlbumYear(int year) {
    HtmlSelect albumYearSelectOption = (HtmlSelect) page.getElementById("albumYear");
    HtmlOption yearOption = albumYearSelectOption.getOptionByValue(Integer.toString(year));
    albumYearSelectOption.setSelectedAttribute(yearOption, true);
    return this;
  }


  
  public AlbumPage changeAlbumRating(String albumRating) {
    HtmlTextInput albumRatingInputTextField = (HtmlTextInput) page.getElementById("albumRating");
    albumRatingInputTextField.setText(albumRating);
    return this;
  }

  
  public AlbumPage changeNumberOfSongs(int numberOfSongs) {
    HtmlNumberInput numberOfSongsNumberField = (HtmlNumberInput) page.getElementById("numberOfSongs");
    numberOfSongsNumberField.setText(Integer.toString(numberOfSongs));
    return this;
  }


  
  public AlbumListPage cancelChanges() {
    HtmlSubmitInput cancelButton = (HtmlSubmitInput) page.getElementById("cancelButton");
    try {
      cancelButton.click();
    } catch (IOException e) {
      e.printStackTrace();
    }
    return new AlbumListPage(webClient);
  }


  
  public AlbumPage saveChanges() {
    HtmlSubmitInput saveButton = (HtmlSubmitInput) page.getElementById("saveButton");
    try {
      saveButton.click();
    } catch (IOException e) {
      e.printStackTrace();
    }
    return this;
  }

}
