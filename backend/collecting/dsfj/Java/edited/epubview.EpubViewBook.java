package com.alibaba.json.test.epubview;

import java.io.Serializable;
import java.util.List;


public class EpubViewBook implements Serializable {
    
    private static final long serialVersionUID = -3450886861177869027L;
    private String bookName;
    private int pageCountNum;
    private boolean teachingFlag = true;
    private List<EpubViewPage> pageList;
    private EpubViewMetaData metadata;
    public String getBookName() {
        return bookName;
    }
    public void setBookName(String bookName) {
        this.bookName = bookName;
    }
    public List<EpubViewPage> getPageList() {
        return pageList;
    }
    public void setPageList(List<EpubViewPage> pageList) {
        this.pageList = pageList;
    }
    
    public int getPageCountNum() {
        return pageCountNum;
    }
    public void setPageCountNum(int pageCountNum) {
        this.pageCountNum = pageCountNum;
    }

    public boolean isTeachingFlag() {
        return teachingFlag;
    }
    
    public void setTeachingFlag(boolean teachingFlag) {
        this.teachingFlag = teachingFlag;
    }
    public EpubViewMetaData getMetadata()
    {
        return metadata;
    }
    public void setMetadata(EpubViewMetaData metadata)
    {
        this.metadata = metadata;
    }
}
