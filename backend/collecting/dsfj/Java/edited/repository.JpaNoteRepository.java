

package sample.jpa.repository;

import java.util.List;

import javax.persistence.EntityManager;
import javax.persistence.PersistenceContext;

import sample.jpa.domain.Note;

import org.springframework.stereotype.Repository;

@Repository
class JpaNoteRepository implements NoteRepository {

	@PersistenceContext
	private EntityManager entityManager;

	@Override
	public List<Note> findAll() {
		return this.entityManager.createQuery("SELECT n FROM Note n", Note.class)
				.getResultList();
	}

}
