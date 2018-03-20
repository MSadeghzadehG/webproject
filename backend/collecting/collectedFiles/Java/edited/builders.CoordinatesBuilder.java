

package org.elasticsearch.common.geo.builders;

import com.vividsolutions.jts.geom.Coordinate;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.List;


public class CoordinatesBuilder {

    private final List<Coordinate> points = new ArrayList<>();

    
    public CoordinatesBuilder coordinate(Coordinate coordinate) {
        this.points.add(coordinate);
        return this;
    }

    
    public CoordinatesBuilder coordinate(double longitude, double latitude) {
        return this.coordinate(new Coordinate(longitude, latitude));
    }

    
    public CoordinatesBuilder coordinates(Coordinate...coordinates) {
        return this.coordinates(Arrays.asList(coordinates));
    }

    
    public CoordinatesBuilder coordinates(Collection<? extends Coordinate> coordinates) {
        this.points.addAll(coordinates);
        return this;
    }

    
    public CoordinatesBuilder close() {
        Coordinate start = points.get(0);
        Coordinate end = points.get(points.size()-1);
        if(start.x != end.x || start.y != end.y) {
            points.add(start);
        }
        return this;
    }

    
    public List<Coordinate> build() {
        return new ArrayList<>(this.points);
    }
}
