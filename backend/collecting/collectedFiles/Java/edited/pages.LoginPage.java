
package com.iluwatar.pageobject.pages;

import com.gargoylesoftware.htmlunit.WebClient;
import com.gargoylesoftware.htmlunit.html.HtmlPage;
import com.gargoylesoftware.htmlunit.html.HtmlPasswordInput;
import com.gargoylesoftware.htmlunit.html.HtmlSubmitInput;
import com.gargoylesoftware.htmlunit.html.HtmlTextInput;
import java.io.IOException;


public class LoginPage extends Page {

  private static final String LOGIN_PAGE_HTML_FILE = "login.html";
  private static final String PAGE_URL = "file:" + AUT_PATH + LOGIN_PAGE_HTML_FILE;

  private HtmlPage page;

  
  public LoginPage(WebClient webClient) {
    super(webClient);
  }

  
  public LoginPage navigateToPage() {
    try {
      page = this.webClient.getPage(PAGE_URL);
    } catch (IOException e) {
      e.printStackTrace();
    }
    return this;
  }

  
  @Override
  public boolean isAt() {
    return "Login".equals(page.getTitleText());
  }


  
  public LoginPage enterUsername(String username) {
    HtmlTextInput usernameInputTextField = (HtmlTextInput) page.getElementById("username");
    usernameInputTextField.setText(username);
    return this;
  }


  
  public LoginPage enterPassword(String password) {
    HtmlPasswordInput passwordInputPasswordField = (HtmlPasswordInput) page.getElementById("password");
    passwordInputPasswordField.setText(password);
    return this;
  }


  
  public AlbumListPage login() {
    HtmlSubmitInput loginButton = (HtmlSubmitInput) page.getElementById("loginButton");
    try {
      loginButton.click();
    } catch (IOException e) {
      e.printStackTrace();
    }
    return new AlbumListPage(webClient);
  }

}
