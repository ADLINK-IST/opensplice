/**
 *                             Vortex Cafe
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
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


import javafx.animation.AnimationTimer;


public class ShapesAnimationTimer
   extends AnimationTimer
{

   private ShapesAnimationThread shapesThread;
   long refreshTime;
   long lastRefreshTime = 0;

   public ShapesAnimationTimer(int refreshTime, ShapesAnimationThread shapesThread)
   {
      // convert refreshTime in ms to nanosec
      this.refreshTime = refreshTime * 1000000;
      this.shapesThread = shapesThread;
   }

   @Override
   public void handle(long now)
   {
      if (now - lastRefreshTime > refreshTime)
      {
         // we just drive the painting code from ShapesAnimationThread
         shapesThread.doPainting();

         lastRefreshTime = now;
      }
   }

}
