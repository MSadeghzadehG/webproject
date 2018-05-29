

package io.reactivex.flowable;

import static org.junit.Assert.*;

import org.junit.Test;

import io.reactivex.*;
import io.reactivex.flowable.FlowableCovarianceTest.*;
import io.reactivex.functions.BiFunction;

public class FlowableReduceTests {

    @Test
    public void reduceIntsFlowable() {
        Flowable<Integer> o = Flowable.just(1, 2, 3);
        int value = o.reduce(new BiFunction<Integer, Integer, Integer>() {
            @Override
            public Integer apply(Integer t1, Integer t2) {
                return t1 + t2;
            }
        }).toFlowable().blockingSingle();

        assertEquals(6, value);
    }

    @SuppressWarnings("unused")
    @Test
    public void reduceWithObjectsFlowable() {
        Flowable<Movie> horrorMovies = Flowable.<Movie> just(new HorrorMovie());

        Flowable<Movie> reduceResult = horrorMovies.scan(new BiFunction<Movie, Movie, Movie>() {
            @Override
            public Movie apply(Movie t1, Movie t2) {
                return t2;
            }
        }).takeLast(1);

        Flowable<Movie> reduceResult2 = horrorMovies.reduce(new BiFunction<Movie, Movie, Movie>() {
            @Override
            public Movie apply(Movie t1, Movie t2) {
                return t2;
            }
        }).toFlowable();

        assertNotNull(reduceResult2);
    }

    
    @Test
    public void reduceWithCovariantObjectsFlowable() {
        Flowable<Movie> horrorMovies = Flowable.<Movie> just(new HorrorMovie());

        Flowable<Movie> reduceResult2 = horrorMovies.reduce(new BiFunction<Movie, Movie, Movie>() {
            @Override
            public Movie apply(Movie t1, Movie t2) {
                return t2;
            }
        }).toFlowable();

        assertNotNull(reduceResult2);
    }


    @Test
    public void reduceInts() {
        Flowable<Integer> o = Flowable.just(1, 2, 3);
        int value = o.reduce(new BiFunction<Integer, Integer, Integer>() {
            @Override
            public Integer apply(Integer t1, Integer t2) {
                return t1 + t2;
            }
        }).toFlowable().blockingSingle();

        assertEquals(6, value);
    }

    @SuppressWarnings("unused")
    @Test
    public void reduceWithObjects() {
        Flowable<Movie> horrorMovies = Flowable.<Movie> just(new HorrorMovie());

        Flowable<Movie> reduceResult = horrorMovies.scan(new BiFunction<Movie, Movie, Movie>() {
            @Override
            public Movie apply(Movie t1, Movie t2) {
                return t2;
            }
        }).takeLast(1);

        Maybe<Movie> reduceResult2 = horrorMovies.reduce(new BiFunction<Movie, Movie, Movie>() {
            @Override
            public Movie apply(Movie t1, Movie t2) {
                return t2;
            }
        });

        assertNotNull(reduceResult2);
    }

    
    @Test
    public void reduceWithCovariantObjects() {
        Flowable<Movie> horrorMovies = Flowable.<Movie> just(new HorrorMovie());

        Maybe<Movie> reduceResult2 = horrorMovies.reduce(new BiFunction<Movie, Movie, Movie>() {
            @Override
            public Movie apply(Movie t1, Movie t2) {
                return t2;
            }
        });

        assertNotNull(reduceResult2);
    }

    
    @Test
    public void reduceCovariance() {
                Flowable<Movie> horrorMovies = Flowable.<Movie> just(new HorrorMovie());
        libraryFunctionActingOnMovieObservables(horrorMovies);
    }

    
    public void libraryFunctionActingOnMovieObservables(Flowable<Movie> obs) {

        obs.reduce(new BiFunction<Movie, Movie, Movie>() {
            @Override
            public Movie apply(Movie t1, Movie t2) {
                return t2;
            }
        });
    }

}
