
package com.iluwatar.model.view.presenter;

import java.io.Serializable;


public class FileSelectorPresenter implements Serializable {

  
  private static final long serialVersionUID = 1210314339075855074L;

  
  private FileSelectorView view;

  
  private FileLoader loader;

  
  public FileSelectorPresenter(FileSelectorView view) {
    this.view = view;
  }

  
  public void setLoader(FileLoader loader) {
    this.loader = loader;
  }

  
  public void start() {
    view.setPresenter(this);
    view.open();
  }

  
  public void fileNameChanged() {
    loader.setFileName(view.getFileName());
  }

  
  public void confirmed() {
    if (loader.getFileName() == null || loader.getFileName().equals("")) {
      view.showMessage("Please give the name of the file first!");
      return;
    }

    if (loader.fileExists()) {
      String data = loader.loadData();
      view.displayData(data);
    } else {
      view.showMessage("The file specified does not exist.");
    }
  }

  
  public void cancelled() {
    view.close();
  }
}
