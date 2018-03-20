

package sample.jpa.repository;

import java.util.List;

import sample.jpa.domain.Note;

public interface NoteRepository {

	List<Note> findAll();

}
