
package com.alibaba.dubbo.validation.support;

import com.alibaba.dubbo.common.URL;
import com.alibaba.dubbo.validation.Validation;
import com.alibaba.dubbo.validation.Validator;

import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.ConcurrentMap;


public abstract class AbstractValidation implements Validation {

    private final ConcurrentMap<String, Validator> validators = new ConcurrentHashMap<String, Validator>();

    public Validator getValidator(URL url) {
        String key = url.toFullString();
        Validator validator = validators.get(key);
        if (validator == null) {
            validators.put(key, createValidator(url));
            validator = validators.get(key);
        }
        return validator;
    }

    protected abstract Validator createValidator(URL url);

}
