
package com.fernandocejas.android10.sample.presentation.presenter;

import android.support.annotation.NonNull;
import com.fernandocejas.android10.sample.domain.User;
import com.fernandocejas.android10.sample.domain.exception.DefaultErrorBundle;
import com.fernandocejas.android10.sample.domain.exception.ErrorBundle;
import com.fernandocejas.android10.sample.domain.interactor.DefaultObserver;
import com.fernandocejas.android10.sample.domain.interactor.GetUserList;
import com.fernandocejas.android10.sample.presentation.exception.ErrorMessageFactory;
import com.fernandocejas.android10.sample.presentation.internal.di.PerActivity;
import com.fernandocejas.android10.sample.presentation.mapper.UserModelDataMapper;
import com.fernandocejas.android10.sample.presentation.model.UserModel;
import com.fernandocejas.android10.sample.presentation.view.UserListView;
import java.util.Collection;
import java.util.List;
import javax.inject.Inject;


@PerActivity
public class UserListPresenter implements Presenter {

  private UserListView viewListView;

  private final GetUserList getUserListUseCase;
  private final UserModelDataMapper userModelDataMapper;

  @Inject
  public UserListPresenter(GetUserList getUserListUserCase,
      UserModelDataMapper userModelDataMapper) {
    this.getUserListUseCase = getUserListUserCase;
    this.userModelDataMapper = userModelDataMapper;
  }

  public void setView(@NonNull UserListView view) {
    this.viewListView = view;
  }

  @Override public void resume() {}

  @Override public void pause() {}

  @Override public void destroy() {
    this.getUserListUseCase.dispose();
    this.viewListView = null;
  }

  
  public void initialize() {
    this.loadUserList();
  }

  
  private void loadUserList() {
    this.hideViewRetry();
    this.showViewLoading();
    this.getUserList();
  }

  public void onUserClicked(UserModel userModel) {
    this.viewListView.viewUser(userModel);
  }

  private void showViewLoading() {
    this.viewListView.showLoading();
  }

  private void hideViewLoading() {
    this.viewListView.hideLoading();
  }

  private void showViewRetry() {
    this.viewListView.showRetry();
  }

  private void hideViewRetry() {
    this.viewListView.hideRetry();
  }

  private void showErrorMessage(ErrorBundle errorBundle) {
    String errorMessage = ErrorMessageFactory.create(this.viewListView.context(),
        errorBundle.getException());
    this.viewListView.showError(errorMessage);
  }

  private void showUsersCollectionInView(Collection<User> usersCollection) {
    final Collection<UserModel> userModelsCollection =
        this.userModelDataMapper.transform(usersCollection);
    this.viewListView.renderUserList(userModelsCollection);
  }

  private void getUserList() {
    this.getUserListUseCase.execute(new UserListObserver(), null);
  }

  private final class UserListObserver extends DefaultObserver<List<User>> {

    @Override public void onComplete() {
      UserListPresenter.this.hideViewLoading();
    }

    @Override public void onError(Throwable e) {
      UserListPresenter.this.hideViewLoading();
      UserListPresenter.this.showErrorMessage(new DefaultErrorBundle((Exception) e));
      UserListPresenter.this.showViewRetry();
    }

    @Override public void onNext(List<User> users) {
      UserListPresenter.this.showUsersCollectionInView(users);
    }
  }
}
