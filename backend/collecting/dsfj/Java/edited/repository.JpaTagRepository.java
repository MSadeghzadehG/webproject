

package sample.jpa.repository;

import java.util.List;

import javax.persistence.EntityManager;
import javax.persistence.PersistenceContext;

import sample.jpa.domain.Tag;

import org.springframework.stereotype.Repository;

@Repository
class JpaTagRepository implements TagRepository {

	@PersistenceContext
	private EntityManager entityManager;

	@Override
	public List<Tag> findAll() {
		return this.entityManager.createQuery("SELECT t FROM Tag t", Tag.class)
				.getResultList();
	}

}
