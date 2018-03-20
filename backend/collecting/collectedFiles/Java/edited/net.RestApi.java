
package com.fernandocejas.android10.sample.data.net;

import com.fernandocejas.android10.sample.data.entity.UserEntity;
import io.reactivex.Observable;
import java.util.List;


public interface RestApi {
  String API_BASE_URL =
      "https:
  
  String API_URL_GET_USER_LIST = API_BASE_URL + "users.json";
  
  String API_URL_GET_USER_DETAILS = API_BASE_URL + "user_";

  
  Observable<List<UserEntity>> userEntityList();

  
  Observable<UserEntity> userEntityById(final int userId);
}
