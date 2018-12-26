package com.alibaba.json.bvtVO.mogujie;

import java.io.Serializable;


public class BankCard implements Serializable {
    private static final long serialVersionUID = -8043292491053382301L;

    public static final Integer CARD_TYPE_DEBIT = 1;            public static final Integer CARD_TYPE_CREDIT = 2;       
    private Long id;
    private String bankId;

    public Long getId() {
        return id;
    }

    public void setId(Long id) {
        this.id = id;
    }


    public String getBankId() {
        return bankId;
    }

    public void setBankId(String bankId) {
        this.bankId = bankId;
    }

}