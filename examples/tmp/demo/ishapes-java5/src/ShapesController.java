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

import java.util.logging.Level;

import javax.script.ScriptEngine;
import javax.script.ScriptEngineManager;
import javax.script.ScriptException;

import org.omg.dds.core.DDSException;
import org.omg.dds.core.policy.QosPolicy.ForDataReader;
import org.omg.dds.core.policy.QosPolicy.ForDataWriter;
import org.omg.dds.pub.DataWriter;
import org.omg.dds.sub.DataReader;

/**
 * Base class of a Controller for iShapes, managing DDS and animation thread. It
 * provides operations to add a publication or a subscription. It also provides
 * an {@link ShapesController#evalScript(String)} operation able to evaluate
 * some JavaScript code.
 */
public abstract class ShapesController {

    private static final Logger log = Logger.getInstance();

    protected ShapesDDSManager shapesDDS;
    protected ShapesAnimationThread shapesAnimThread;

    protected ShapesPainter shapesPainter;

    public void initDDS() {
        // init DDS Manager
        try {
            shapesDDS = new ShapesDDSManager();
            log.log("initialize shapesDDS", Level.INFO);
            shapesDDS.init();
        } catch (DDSException e) {
            log.log("Error initializing DDS: " + e + " caused by "
                    + e.getCause(), Level.SEVERE);
            // re-throw for full stack trace in logs
            throw e;
        }
    }

    public void closeDDS() {
        // close DDS
        log.log("Close DDS", Level.INFO);
        shapesDDS.close();
    }

    public void startAnimationThread(int refreshTime) {
        // init and start ShapesAnimationThread
        shapesAnimThread = new ShapesAnimationThread(refreshTime, shapesPainter);
        shapesAnimThread.start();
    }

    public void waitForAnimationThread() {
        try {
            shapesAnimThread.join();
        } catch (InterruptedException e) {
            // ignore
        }
    }

    public void stopAnimationThread() {
        // terminate animation thread
        log.log("Stop ShapesAnimationThread", Level.INFO);
        shapesAnimThread.terminate();
    }

    public void addPublication(ShapeKind pubShapeKind, String color,
            ForDataWriter... qos) {
        addPublication(pubShapeKind, color, ShapesConstants.DEFAULT_SHAPE_SIZE,
                ShapesConstants.DEFAULT_SHAPE_SPEED, qos);
    }

    public void addPublication(ShapeKind pubShapeKind, String color, int size,
            int speed, ForDataWriter... qos) {
        // get DDS DataWriter
        log.log("get DDS DataWriter for " + pubShapeKind, Level.INFO);
        DataWriter<ShapeType> dw = shapesDDS.getDataWriter(pubShapeKind, qos);

        // add new BouncingShape to ShapesAnimationThread
        log.log("add " + color + " " + pubShapeKind + " BouncingShape",
                Level.INFO);
        shapesAnimThread.add(new BouncingShape(pubShapeKind, color, size,
                speed, dw, shapesPainter));
    }

    public void addSubscription(ShapeKind subShapeKind, ForDataReader... qos) {
        addSubscription(subShapeKind, null, qos);
    }

    public void addSubscription(ShapeKind subShapeKind,
            Filter<ShapeType> filter, ForDataReader... qos) {
        // get DDS DataReader
        log.log("get DDS DataReader for " + subShapeKind, Level.INFO);
        DataReader<ShapeType> dr = shapesDDS.getDataReader(subShapeKind, qos);

        // add new ShapesDataReader to ShapesManager
        log.log("add ShapesDataReader for " + subShapeKind, Level.INFO);
        shapesAnimThread.add(new ShapesDataReader(subShapeKind, dr,
                shapesPainter, filter));
    }

    public void evalScript(String script) {
        // create a script engine manager
        ScriptEngineManager factory = new ScriptEngineManager();
        // create a JavaScript engine
        ScriptEngine engine = factory.getEngineByName("JavaScript");
        if (engine != null) {
            // add this ShapesController as "controller" variable
            engine.put("controller", this);
            // add PolicyFactory as "policyFactory" variable
            engine.put("policyFactory", shapesDDS.getPolicyFactory());
            // add ShapeKinds for convenience
            engine.put("CIRCLE", ShapeKind.CIRCLE);
            engine.put("SQUARE", ShapeKind.SQUARE);
            engine.put("TRIANGLE", ShapeKind.TRIANGLE);

            // evaluate JavaScript code from String
            try {
                engine.eval(script);
            } catch (ScriptException e) {
                e.printStackTrace();
            }
        } else {
            log.log("Unable to find 'JavaScript' engine", Level.WARNING);
        }
    }

}
