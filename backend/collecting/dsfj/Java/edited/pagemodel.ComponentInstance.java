package com.alibaba.json.test.entity.pagemodel;


public abstract class ComponentInstance {

    protected Long   sid;
    protected String cid;

    public Long getSid() {
        return sid;
    }

    public void setSid(Long sid) {
        this.sid = sid;
    }

    public String getCid() {
        return cid;
    }

    public void setCid(String cid) {
        this.cid = cid;
    }

}
