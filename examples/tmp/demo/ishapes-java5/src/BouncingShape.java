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


import java.util.Random;
import java.util.concurrent.TimeoutException;
import java.util.logging.Level;

import org.omg.dds.pub.DataWriter;


public class BouncingShape
{

    static private final Logger log = Logger.getInstance();

   /** shape kind */
   private ShapeKind kind;

   /** shape data */
   private ShapeType data;

   /** absolute speed of the shape */
   private int speed;
   /** x speed of the shape */
   private int xSpeed;
   /** y speed of the shape */
   private int ySpeed;
   /** x direction of the shape (true=positive) */
   private boolean xDirection;
   /** y direction of the shape (true=positive) */
   private boolean yDirection;

   /** DDS DataWriter for publication */
   private DataWriter<ShapeType> writer;

   /** ShapePainter for display */
   private ShapesPainter painter;

   private Random random = new Random();


   public BouncingShape(ShapeKind kind,
      String color,
      int size,
      int speed,
      DataWriter<ShapeType> writer,
      ShapesPainter painter)
   {
      this.painter = painter;
      this.kind = kind;
      this.data = newRandomShape(size, color);
      this.speed = speed;
      this.writer = writer;

      // set random direction and angle-speed
      xDirection = random.nextBoolean();
      yDirection = random.nextBoolean();
      changeDirection();
   }

   // X-axis limit (i.e. right wall)
   private int getXLimit()
   {
      return painter.getXLimit() - data.shapesize;
   }

   // Y-axis limit (i.e. bottom wall)
   private int getYLimit()
   {
      return painter.getYLimit() - data.shapesize;
   }

   private ShapeType newRandomShape(int size, String color)
   {
        log.log("set random position within (0-" + painter.getXLimit()
                + " , 0-" + painter.getYLimit() + ") range", Level.FINER);
      return new ShapeType(color,
            random.nextInt(painter.getXLimit() - size),
            random.nextInt(painter.getYLimit() - size),
            size);
   }

   private void changeDirection()
   {
      double angleSpeed = (random.nextDouble() + 1) / 2;

      xSpeed = (int) Math.round(angleSpeed * speed);
      if ( !xDirection)
         xSpeed = -xSpeed;
      ySpeed = (int) Math.round(angleSpeed * speed);
      if ( !yDirection)
         ySpeed = -ySpeed;
   }

   public void updatePosition()
   {
      data.x = data.x + xSpeed;
      data.y = data.y + ySpeed;

      // test if x is out-of-bounds
      if (data.x <= 0)
      {
         xDirection = true;
         changeDirection();
         data.x = 0;
      }
      else if (data.x >= getXLimit())
      {
         xDirection = false;
         changeDirection();
         data.x = getXLimit();
      }

      // test if y is out-of-bounds
      if (data.y <= 0)
      {
         yDirection = true;
         changeDirection();
         data.y = 0;
      }
      else if (data.y >= getYLimit())
      {
         yDirection = false;
         changeDirection();
         data.y = getYLimit();
      }
   }


   public void publish()
   {
      try
      {
            log.log("Publish " + data.color + " kind " + " shape", Level.FINE);
         writer.write(data);
      }
      catch (TimeoutException e)
      {
            log.log("TimeoutException publishing a " + data.color + " kind "
                    + " shape", Level.SEVERE);
      }
   }

   public void paint()
   {
      switch (kind)
      {
         case CIRCLE:
            painter.addCircle(data, true);
            break;
         case SQUARE:
            painter.addSquare(data, true);
            break;
         case TRIANGLE:
            painter.addTriangle(data, true);
            break;
      }
   }

}
