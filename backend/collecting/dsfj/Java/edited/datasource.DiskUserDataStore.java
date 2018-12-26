
package com.fernandocejas.android10.sample.data.repository.datasource;

import com.fernandocejas.android10.sample.data.cache.UserCache;
import com.fernandocejas.android10.sample.data.entity.UserEntity;
import io.reactivex.Observable;
import java.util.List;


class DiskUserDataStore implements UserDataStore {

  private final UserCache userCache;

  
  DiskUserDataStore(UserCache userCache) {
    this.userCache = userCache;
  }

  @Override public Observable<List<UserEntity>> userEntityList() {
        throw new UnsupportedOperationException("Operation is not available!!!");
  }

  @Override public Observable<UserEntity> userEntityDetails(final int userId) {
     return this.userCache.get(userId);
  }
}
