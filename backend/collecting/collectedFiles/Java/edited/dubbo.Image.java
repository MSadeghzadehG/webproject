
package com.alibaba.json.test.dubbo;

import java.io.InputStream;
import java.io.Serializable;


public class Image implements Serializable {
    
    private static final long serialVersionUID = 616779453943392868L;
    String                    name;
    InputStream               is;

    public String getName() {
        return name;
    }

    public void setName(String name) {
        this.name = name;
    }

    public InputStream getIs() {
        return is;
    }

    public void setIs(InputStream is) {
        this.is = is;
    }

}
