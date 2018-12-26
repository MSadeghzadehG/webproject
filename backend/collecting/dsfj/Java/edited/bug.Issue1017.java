package com.alibaba.json.bvt.bug;

import com.alibaba.fastjson.JSON;
import junit.framework.TestCase;

import java.io.Serializable;
import java.util.List;


public class Issue1017 extends TestCase {
    public void test_for_issue() throws Exception {
        String json = "{\"pictureList\":[\"http:
        User user = JSON.parseObject(json, User.class);
        assertNotNull(user.pictureList);
        assertEquals(2, user.pictureList.size());
        assertEquals("http:        assertEquals("http:    }

    public static class User implements Serializable {
        private List<String> pictureList;
        public List<String> getPictureList() {
            return pictureList;
        }
        public User setPictureList(List<String> pictureList) {
            this.pictureList = pictureList;
            return this;
        }
    }
}
