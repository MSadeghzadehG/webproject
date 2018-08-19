
package com.iluwatar.model.view.presenter;

import java.io.Serializable;


public interface FileSelectorView extends Serializable {

  
  void open();

  
  void close();

  
  boolean isOpened();

  
  void setPresenter(FileSelectorPresenter presenter);

  
  FileSelectorPresenter getPresenter();

  
  void setFileName(String name);

  
  String getFileName();

  
  void showMessage(String message);

  
  void displayData(String data);
}
