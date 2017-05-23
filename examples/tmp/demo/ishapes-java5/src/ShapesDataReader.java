/**
 *                             Vortex Cafe
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited, its affiliated companies and licensors. All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 */

import java.io.IOException;
import java.util.Random;
import java.util.logging.Level;

import org.omg.dds.sub.DataReader;
import org.omg.dds.sub.DataReader.Selector;
import org.omg.dds.sub.Sample.Iterator;

public class ShapesDataReader {

    static private final Logger log = Logger.getInstance();

    private ShapeKind shapeKind;
    private DataReader<ShapeType> reader;
    private ShapesPainter painter;
    private Selector<ShapeType> selector;

    private ShapeType defaultShape;

    private static final Random random = new Random();

    public ShapesDataReader(ShapeKind shapeKind, DataReader<ShapeType> reader,
            ShapesPainter painter, Filter<ShapeType> filter) {
        this.shapeKind = shapeKind;
        this.reader = reader;
        this.painter = painter;
        if (filter != null) {
            log.log("Filter expression: " + filter.getExpression() + "\n",
                    Level.INFO);
            this.selector = reader.select().Content(filter.getExpression(),
                    filter.getParameters());
        } else {
            this.selector = reader.select();
        }

        // default Shape displayed when nothing was read
        defaultShape = new ShapeType("GREY", random.nextInt(painter.getXLimit()
                - ShapesConstants.DEFAULT_SHAPE_SIZE), random.nextInt(painter
                .getYLimit() - ShapesConstants.DEFAULT_SHAPE_SIZE),
                ShapesConstants.DEFAULT_SHAPE_SIZE);
    }

    public void readAndPaint() {
        log.log("read for " + shapeKind + " shapes", Level.FINE);
        int count = 0;
        Iterator<ShapeType> it = reader.read(selector);
        try {
            for (; it.hasNext(); count++) {
                ShapeType shape = it.next().getData();
                if (shape != null) {
                    switch (shapeKind) {
                    case CIRCLE:
                        painter.addCircle(shape, false);
                        break;
                    case SQUARE:
                        painter.addSquare(shape, false);
                        break;
                    case TRIANGLE:
                        painter.addTriangle(shape, false);
                        break;
                    }
                    // save position and size in defaultShape for a
                    // "last value" effect in case we receive no more shape
                    defaultShape.x = shape.x;
                    defaultShape.y = shape.y;
                    defaultShape.shapesize = shape.shapesize;
                }
            }
        } finally {
            try {
                it.close();
            } catch (IOException e) {
                log.log("Exception closing iterator " + e, Level.WARNING);
            }
        }
        log.log(count + " " + shapeKind + " shapes added", Level.FINE);

        if (count == 0) {
            switch (shapeKind) {
            case CIRCLE:
                painter.addCircle(defaultShape, false);
                break;
            case SQUARE:
                painter.addSquare(defaultShape, false);
                break;
            case TRIANGLE:
                painter.addTriangle(defaultShape, false);
                break;
            }
        }
    }
}
