

package com.alibaba.com.caucho.hessian.io;


public class HessianRemote {
    private String type;
    private String url;

    
    public HessianRemote(String type, String url) {
        this.type = type;
        this.url = url;
    }

    
    public HessianRemote() {
    }

    
    public String getType() {
        return type;
    }

    
    public String getURL() {
        return url;
    }

    
    public void setURL(String url) {
        this.url = url;
    }

    
    public int hashCode() {
        return url.hashCode();
    }

    
    public boolean equals(Object obj) {
        if (!(obj instanceof HessianRemote))
            return false;

        HessianRemote remote = (HessianRemote) obj;

        return url.equals(remote.url);
    }

    
    public String toString() {
        return "[HessianRemote " + url + "]";
    }
}
