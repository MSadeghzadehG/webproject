
package com.alibaba.dubbo.rpc.protocol.rest;

import javax.xml.bind.annotation.XmlAccessType;
import javax.xml.bind.annotation.XmlAccessorType;
import javax.xml.bind.annotation.XmlRootElement;
import java.io.Serializable;

@XmlRootElement(name = "constraintViolation")
@XmlAccessorType(XmlAccessType.FIELD)
public class RestConstraintViolation implements Serializable {

    private static final long serialVersionUID = -23497234978L;

    private String path;
    private String message;
    private String value;

    public RestConstraintViolation(String path, String message, String value) {
        this.path = path;
        this.message = message;
        this.value = value;
    }

    public RestConstraintViolation() {
    }

    public String getPath() {
        return path;
    }

    public void setPath(String path) {
        this.path = path;
    }

    public String getMessage() {
        return message;
    }

    public void setMessage(String message) {
        this.message = message;
    }

    public String getValue() {
        return value;
    }

    public void setValue(String value) {
        this.value = value;
    }
}
