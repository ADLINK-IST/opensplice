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


import java.util.Hashtable;
import java.util.Locale;
import java.util.Map;
import java.util.Map.Entry;

import javafx.scene.Node;
import javafx.scene.image.ImageView;
import javafx.scene.layout.Pane;
import javafx.scene.paint.Color;
import javafx.scene.shape.Circle;
import javafx.scene.shape.Polygon;
import javafx.scene.shape.Rectangle;


public class JavaFXShapesPainter
   implements ShapesPainter
{

   private Pane shapesPanel;
   private ImageView logo;

   private Map<String, Color> colors;

   public JavaFXShapesPainter(Pane shapesPanel, ImageView logo)
   {
      this.shapesPanel = shapesPanel;
      this.logo = logo;

      // convert color codes to JavaFX Color objects
      colors = new Hashtable<String, Color>();
        for (Entry<String, Integer> entry : ShapesConstants.COLORS_CODES
                .entrySet())
      {
         colors.put(entry.getKey(),
               Color.rgb(
                     (entry.getValue() & 0xFF0000) >> 16,
                     (entry.getValue() & 0x00FF00) >> 8,
                     (entry.getValue() & 0x0000FF)));
      }
   }

   @Override
   public int getXLimit()
   {
      return (int) shapesPanel.getWidth();
   }

   @Override
   public int getYLimit()
   {
      return (int) shapesPanel.getHeight();
   }

   @Override
   public void addCircle(ShapeType shape, boolean whiteTag)
   {
      Circle circle = new Circle(shape.x + shape.shapesize / 2.0, shape.y + shape.shapesize / 2.0,
            shape.shapesize / 2.0);
      circle.setStroke(colors.get("white"));
      circle.setFill(colors.get(shape.color.toLowerCase(Locale.US)));
      addToShapePanel(circle);

      addTag(shape.x + shape.shapesize / 2,
            shape.y + shape.shapesize / 2,
            shape.shapesize / 3,
            whiteTag);
   }

   @Override
   public void addSquare(ShapeType shape, boolean whiteTag)
   {
      Rectangle square = new Rectangle(shape.x, shape.y, shape.shapesize, shape.shapesize);
      square.setStroke(colors.get("white"));
      square.setFill(colors.get(shape.color.toLowerCase(Locale.US)));
      addToShapePanel(square);

      addTag(shape.x + shape.shapesize / 2,
            shape.y + shape.shapesize / 2,
            shape.shapesize / 3,
            whiteTag);
   }

   @Override
   public void addTriangle(ShapeType shape, boolean whiteTag)
   {
      Polygon triangle = new Polygon(
            shape.x + shape.shapesize / 2, shape.y,
            shape.x + shape.shapesize, shape.y + shape.shapesize,
            shape.x, shape.y + shape.shapesize
            );
      triangle.setStroke(colors.get("white"));
      triangle.setFill(colors.get(shape.color.toLowerCase(Locale.US)));
      addToShapePanel(triangle);

      addTag(shape.x + shape.shapesize / 2,
            shape.y + 2 * (shape.shapesize / 3),
            shape.shapesize / 3,
            whiteTag);
   }

   private void addTag(int xCenter, int yCenter, int size, boolean whiteTag)
   {
      Circle tag = new Circle(xCenter, yCenter, size / 2.0,
            (whiteTag ? colors.get("white") : colors.get("black")));
      addToShapePanel(tag);
   }

   private void addToShapePanel(final Node e)
   {
      shapesPanel.getChildren().add(e);
   }

   @Override
   public void paintAll()
   {
      // do nothing (re-paint automatically done by JavaFX)
   }

   @Override
   public void clear()
   {
      shapesPanel.getChildren().clear();
      // add the logo again since it just have been cleared
      shapesPanel.getChildren().add(logo);
   }

}
