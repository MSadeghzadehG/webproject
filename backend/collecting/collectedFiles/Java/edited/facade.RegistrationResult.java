
package com.alibaba.dubbo.examples.rest.api.facade;

import javax.xml.bind.annotation.XmlRootElement;
import java.io.Serializable;


@XmlRootElement
public class RegistrationResult implements Serializable {

    private Long id;

    public RegistrationResult() {
    }

    public RegistrationResult(Long id) {
        this.id = id;
    }

    public Long getId() {
        return id;
    }

    public void setId(Long id) {
        this.id = id;
    }
}
