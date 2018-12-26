
package com.alibaba.json.bvtVO;

import java.io.Serializable;


public class Image implements Serializable {

    private static final long serialVersionUID = -6804500330834961534L;
    private String            imageUrl;

    public Image(String imageUrl){
        super();
        this.imageUrl = imageUrl;
    }

    public Image(){

    }

    public String getBigImageUrl() {
        if (imageUrl == null || imageUrl.length() == 0) {
            return "";
        }
        return ("img/" + imageUrl).replaceFirst(".jpg", ".310x310.jpg");
    }

    
    public String getSearchImageUrl() {
        if (imageUrl == null || imageUrl.length() == 0) {
            return "";
        }
        return ("img/" + imageUrl).replaceFirst(".jpg", ".search.jpg");
    }

    
    public String getSummImageUrl() {
        if (imageUrl == null || imageUrl.length() == 0) {
            return "";
        }
        return ("img/" + imageUrl).replaceFirst(".jpg", ".summ.jpg");
    }

    public String getImageUrl() {
        return imageUrl;
    }

    public void setImageUrl(String imageUrl) {
        this.imageUrl = imageUrl;
    }

}
