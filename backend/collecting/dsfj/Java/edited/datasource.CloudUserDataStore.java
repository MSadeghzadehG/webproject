
package com.fernandocejas.android10.sample.data.repository.datasource;

import com.fernandocejas.android10.sample.data.cache.UserCache;
import com.fernandocejas.android10.sample.data.entity.UserEntity;
import com.fernandocejas.android10.sample.data.net.RestApi;
import io.reactivex.Observable;
import java.util.List;


class CloudUserDataStore implements UserDataStore {

  private final RestApi restApi;
  private final UserCache userCache;

  
  CloudUserDataStore(RestApi restApi, UserCache userCache) {
    this.restApi = restApi;
    this.userCache = userCache;
  }

  @Override public Observable<List<UserEntity>> userEntityList() {
    return this.restApi.userEntityList();
  }

  @Override public Observable<UserEntity> userEntityDetails(final int userId) {
    return this.restApi.userEntityById(userId).doOnNext(CloudUserDataStore.this.userCache::put);
  }
}
