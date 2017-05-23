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
import java.util.Map;


class ShapesConstants
{

   static final String TYPE_NAME_TO_REGISTER =
         System.getProperty("dds.registerType", "ShapeType");
   static final int MINIMUM_SHAPE_SIZE = 10;
   static final int MAXIMUM_SHAPE_SIZE = 100;
   static final int DEFAULT_SHAPE_SIZE = 50;
   static final int MINIMUM_SHAPE_SPEED = 1;
   static final int MAXIMUM_SHAPE_SPEED = 10;
   static final int DEFAULT_SHAPE_SPEED = 3;
   static final int REFRESH_TIMEOUT =
         Integer.parseInt(System.getProperty("refreshTimeout", "0"));
   static final int SHAPE_PANEL_WIDTH = 321;
   static final int SHAPE_PANEL_HEIGHT = 361;

   static final Map<String, Integer> COLORS_CODES = new Hashtable<String, Integer>();

   static
   {
      COLORS_CODES.put("black", 0x000000);
      COLORS_CODES.put("red", 0xcc3333);
      COLORS_CODES.put("green", 0x99cc66);
      COLORS_CODES.put("blue", 0x336699);
      COLORS_CODES.put("orange", 0xff9933);
      COLORS_CODES.put("yellow", 0xffff66);
      COLORS_CODES.put("magenta", 0xcc99cc);
      COLORS_CODES.put("cyan", 0x99ccff);
      COLORS_CODES.put("dark grey", 0x666666);
      COLORS_CODES.put("grey", 0x999999);
      COLORS_CODES.put("white", 0xffffff);
   }

}
