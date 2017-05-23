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

/**
 * ShapesPainter for command-line mode.
 * It displays shapes characteristics on standard output.
 */
public class CmdlineShapesPainter
   implements ShapesPainter
{

   private boolean noPrint;

   public CmdlineShapesPainter(boolean noPrint)
   {
      this.noPrint = noPrint;
   }

   @Override
   public int getXLimit()
   {
      return ShapesConstants.SHAPE_PANEL_WIDTH;
   }

   @Override
   public int getYLimit()
   {
      return ShapesConstants.SHAPE_PANEL_HEIGHT;
   }

   private void printShape(String shapeKind, ShapeType shape, boolean published)
   {
      if ( !noPrint)
      {
         System.out.printf("  %10s  %8s  %8s (size=%2d)  at  (%3d,%3d)%n",
               published ? "PUBLISHED  " : "SUBSCRIBED ",
               shapeKind,
               shape.color,
               shape.shapesize,
               shape.x,
               shape.y);
      }
   }

   @Override
   public void addCircle(ShapeType shape, boolean whiteTag)
   {
      printShape("Circle", shape, whiteTag);
   }

   @Override
   public void addSquare(ShapeType shape, boolean whiteTag)
   {
      printShape("Square", shape, whiteTag);
   }

   @Override
   public void addTriangle(ShapeType shape, boolean whiteTag)
   {
      printShape("Triangle", shape, whiteTag);
   }

   @Override
   public void paintAll()
   {
      // do nothing
   }

   @Override
   public void clear()
   {
      if ( !noPrint)
      {
         System.out.println("---------------------------------------------------------\n\n");
      }
   }

}
