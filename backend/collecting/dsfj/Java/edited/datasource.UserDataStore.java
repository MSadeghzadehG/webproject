
package com.fernandocejas.android10.sample.data.repository.datasource;

import com.fernandocejas.android10.sample.data.entity.UserEntity;
import io.reactivex.Observable;
import java.util.List;


public interface UserDataStore {
  
  Observable<List<UserEntity>> userEntityList();

  
  Observable<UserEntity> userEntityDetails(final int userId);
}
