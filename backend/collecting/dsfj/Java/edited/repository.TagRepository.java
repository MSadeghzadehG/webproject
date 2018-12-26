

package sample.jpa.repository;

import java.util.List;

import sample.jpa.domain.Tag;

public interface TagRepository {

	List<Tag> findAll();

}
