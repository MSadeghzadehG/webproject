
package com.alibaba.dubbo.rpc.protocol.rest;

import javax.xml.bind.annotation.XmlAccessType;
import javax.xml.bind.annotation.XmlAccessorType;
import javax.xml.bind.annotation.XmlRootElement;
import java.io.Serializable;
import java.util.LinkedList;
import java.util.List;

@XmlRootElement(name="violationReport")
@XmlAccessorType(XmlAccessType.FIELD)
public class ViolationReport implements Serializable {

    private static final long serialVersionUID = -130498234L;

    private List<RestConstraintViolation> constraintViolations;

    public List<RestConstraintViolation> getConstraintViolations() {
        return constraintViolations;
    }

    public void setConstraintViolations(List<RestConstraintViolation> constraintViolations) {
        this.constraintViolations = constraintViolations;
    }

    public void addConstraintViolation(RestConstraintViolation constraintViolation) {
        if (constraintViolations == null) {
            constraintViolations = new LinkedList<RestConstraintViolation>();
        }
        constraintViolations.add(constraintViolation);
    }
}
