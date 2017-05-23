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

import java.util.List;
import java.util.concurrent.CopyOnWriteArrayList;
import java.util.logging.Level;

public class ShapesAnimationThread extends Thread {

    static private final Logger log = Logger.getInstance();

    private int refreshRate;
    private boolean terminate = false;
    private boolean paused = false;
    private Object suspendLock = new Object();

    private ShapesPainter shapePainter;

    private List<BouncingShape> publishedShapes;

    private List<ShapesDataReader> shapeReaders;

    public ShapesAnimationThread(ShapesPainter shapePainter) {
        this(ShapesConstants.REFRESH_TIMEOUT, shapePainter);
    }

    public ShapesAnimationThread(int refreshRate, ShapesPainter shapePainter) {
        this.refreshRate = refreshRate;
        this.shapePainter = shapePainter;
        publishedShapes = new CopyOnWriteArrayList<BouncingShape>();
        shapeReaders = new CopyOnWriteArrayList<ShapesDataReader>();
    }

    public void add(BouncingShape bouncingShape) {
        publishedShapes.add(bouncingShape);
    }

    public void add(ShapesDataReader shapeReader) {
        shapeReaders.add(shapeReader);
    }

    public void doPainting() {
        shapePainter.clear();

        // paint bouncing shapes (before read shapes)
        for (BouncingShape shape : publishedShapes) {
            shape.paint();
        }

        // read shapes from DataReaders and paint each
        // (those will overrride published shapes)
        for (ShapesDataReader reader : shapeReaders) {
            reader.readAndPaint();
        }

        shapePainter.paintAll();

        // update and publish bouncing shapes
        // (we do this at end of loop so if this process do publish AND
        // subscribe,
        // the published shape will be received at next loop.
        // Thus, there will be no shift for a same published/read shape)
        for (BouncingShape shape : publishedShapes) {
            shape.updatePosition();
            shape.publish();
        }
    }

    public synchronized void terminate() {
        if (!terminate) {
            paused = true;
            terminate = true;
            try {
                log.log("interrupting and wait for termination...", Level.INFO);
                interrupt();
                join();
            } catch (InterruptedException e) {
                // ignore
                log.log("interruped waiting for termination...", Level.INFO);
            }
            log.log("terminated", Level.INFO);
        }
    }

    public void pause() {
        paused = true;
    }

    public void unpause() {
        paused = false;
        synchronized (suspendLock) {
            suspendLock.notifyAll();
        }
    }

    @Override
    public void run() {
        log.log("running", Level.INFO);
        terminate = false;
        while (!terminate) {
            while (!paused) {
                try {
                    // pain all shapes and update positions of published shapes
                    doPainting();

                    // wait for refresh
                    sleep(refreshRate);

                } catch (InterruptedException e) {
                    log.log("interrupted!", Level.WARNING);
                    terminate = true;
                }
            }

            synchronized (suspendLock) {
                if (paused && !terminate) {
                    try {
                        log.log("paused!", Level.WARNING);
                        suspendLock.wait();
                    } catch (InterruptedException e) {
                        log.log("pause interrupted!", Level.WARNING);
                    }
                    log.log("resumed!", Level.WARNING);
                }
            }

        }
        log.log("end of run", Level.WARNING);
    }

}
