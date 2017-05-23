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


import javafx.application.Application;
import javafx.fxml.FXMLLoader;
import javafx.scene.Scene;
import javafx.scene.image.Image;
import javafx.scene.layout.AnchorPane;
import javafx.stage.Stage;


public class IShapesApplication
   extends Application
{

   private static final String APP_TITLE = "iShapes using JavaFX";
   private static final String FXML_FILE = "/iShapes.fxml";

   private ShapesController controller;

   @Override
   public void start(Stage stage) throws Exception
   {
      // start JavaFX controller
      FXMLLoader fxmlLoader = new FXMLLoader(getClass().getResource(FXML_FILE));
      AnchorPane anchor = (AnchorPane) fxmlLoader.load();
      controller = fxmlLoader.getController();

      stage.setTitle(APP_TITLE);
      try
      {
         stage.getIcons().add(
               new Image(
                     IShapesApplication.class.getClassLoader().getResourceAsStream(
                           "IShapesTransparentLogo32.png")));
      }
      catch (Throwable t)
      {
         // nothing
      }
      stage.setScene(new Scene(anchor));

      // set limit size ow Window (blocking height size)
      stage.setMinWidth(anchor.getPrefWidth());
      stage.setMinHeight(anchor.getPrefHeight() + 30); // add height of window title
      stage.setMaxHeight(stage.getMinHeight());
      stage.show();

      // is a script was provided evaluate it
      String script = IShapesMain.getScript();
      if (script != null)
      {
         controller.evalScript(script);
      }
   }

   @Override
   public void stop() throws Exception
   {
      controller.stopAnimationThread();
      controller.closeDDS();
      super.stop();
   }

}
