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


import java.net.URL;
import java.util.Locale;
import java.util.ResourceBundle;
import java.util.logging.Level;

import javafx.collections.FXCollections;
import javafx.event.ActionEvent;
import javafx.fxml.FXML;
import javafx.fxml.Initializable;
import javafx.scene.control.ChoiceBox;
import javafx.scene.control.Slider;
import javafx.scene.image.ImageView;
import javafx.scene.layout.AnchorPane;
import javafx.scene.layout.Pane;


public class JavaFxController
   extends ShapesController
   implements Initializable
{

   private static final Logger log =
 Logger.getInstance();

   @FXML
   private Pane shapesPanel;
   @FXML
   private ImageView logo;

   @FXML
   private ChoiceBox<String> pubShapeBox;
   @FXML
   private ChoiceBox<String> pubColorBox;
   @FXML
   private Slider pubSizeSlider;
   @FXML
   private Slider pubSpeedSlider;

   @FXML
   private ChoiceBox<String> subShapeBox;

   @FXML
   private AnchorPane publisherQoSPane;
   @FXML
   private PublisherQoSController publisherQoSPaneController;

   @FXML
   private AnchorPane subscriberQoSPane;
   @FXML
   private SubscriberQoSController subscriberQoSPaneController;

   @FXML
   private AnchorPane filterPane;
   @FXML
   private FilterController filterPaneController;

   private ShapesAnimationTimer shapesAnimTimer;

   @Override
   public void initialize(URL fxmlFileLocation, ResourceBundle resources)
   {
      assert shapesPanel != null : "fx:id=\"shapesPanel\" was not injected: check your FXML file 'iShapes.fxml'.";
      assert logo != null : "fx:id=\"logo\" was not injected: check your FXML file 'iShapes.fxml'.";
      assert pubColorBox != null : "fx:id=\"pubColorBox\" was not injected: check your FXML file 'iShapes.fxml'.";
      assert pubShapeBox != null : "fx:id=\"pubShapeBox\" was not injected: check your FXML file 'iShapes.fxml'.";
      assert pubSizeSlider != null : "fx:id=\"pubSizeSlider\" was not injected: check your FXML file 'iShapes.fxml'.";
      assert pubSpeedSlider != null : "fx:id=\"pubSpeedSlider\" was not injected: check your FXML file 'iShapes.fxml'.";
      assert publisherQoSPane != null : "fx:id=\"publisherQoSPane\" was not injected: check your FXML file 'iShapes.fxml'.";
      assert subShapeBox != null : "fx:id=\"subShapeBox\" was not injected: check your FXML file 'iShapes.fxml'.";
      assert subscriberQoSPane != null : "fx:id=\"subscriberQoSPane\" was not injected: check your FXML file 'iShapes.fxml'.";
      assert filterPane != null : "fx:id=\"filterPane\" was not injected: check your FXML file 'iShapes.fxml'.";

      // set pubShapeBox choices
      pubShapeBox.setItems(FXCollections.observableArrayList(
            ShapeKind.CIRCLE.toString().toLowerCase(),
            ShapeKind.SQUARE.toString().toLowerCase(),
            ShapeKind.TRIANGLE.toString().toLowerCase()));
      pubShapeBox.getSelectionModel().select(0);

      // set pubColorBox choices
      pubColorBox.setItems(FXCollections.observableArrayList(
            "blue", "red", "green", "orange", "yellow", "magenta", "cyan"));
      pubColorBox.getSelectionModel().select(0);

      // set SubShapeBox choices
      subShapeBox.setItems(FXCollections.observableArrayList(
            ShapeKind.CIRCLE.toString().toLowerCase(),
            ShapeKind.SQUARE.toString().toLowerCase(),
            ShapeKind.TRIANGLE.toString().toLowerCase()));
      subShapeBox.getSelectionModel().select(0);

      // init DDS
      initDDS();

      // give ShapesDDSManager to PublisherQoSController and SubscriberQoSController
      // and make them save their default QoS values
      publisherQoSPaneController.setShapesDDSManager(shapesDDS);
      publisherQoSPaneController.saveQoS();
      subscriberQoSPaneController.setShapesDDSManager(shapesDDS);
      subscriberQoSPaneController.saveQoS();

      // init ShapesPainter
      shapesPainter = new JavaFXShapesPainter(shapesPanel, logo);

      // init and start ShapesAnimationThread
        startAnimationThread(ShapesConstants.REFRESH_TIMEOUT);
   }


   @Override
   public void startAnimationThread(int refreshTime)
   {
      // For JavaFX we start a ShapesAnimationTimer instead of ShapesAnimationThread
      this.shapesAnimThread = new ShapesAnimationThread(shapesPainter);
      this.shapesAnimTimer = new ShapesAnimationTimer(refreshTime, this.shapesAnimThread);
      shapesAnimTimer.start();
   }

   @Override
   public void stopAnimationThread()
   {
      // stop the ShapesAnimationTimer
      shapesAnimTimer.stop();
   }

   @FXML
   protected void onPubQoSButton(ActionEvent event)
   {
        log.log("onPubQoSButton", Level.INFO);
      publisherQoSPane.setVisible(true);
   }

   @FXML
   protected void onPublishButton(ActionEvent event)
   {
      ShapeKind pubShapeKind = ShapeKind.valueOf(
            pubShapeBox.getSelectionModel().getSelectedItem().toUpperCase());

      // add publication
      addPublication(pubShapeKind,
            pubColorBox.getSelectionModel().getSelectedItem().toUpperCase(Locale.US),
            (int) pubSizeSlider.getValue(),
            (int) pubSpeedSlider.getValue(),
            publisherQoSPaneController.getQoS());
   }

   @FXML
   protected void onSubQoSButton(ActionEvent event)
   {
        log.log("onSubQoSButton", Level.INFO);
      subscriberQoSPane.setVisible(true);
   }

   @FXML
   protected void onSubFilterButton(ActionEvent event)
   {
        log.log("onSubFilterButton:", Level.INFO);
      filterPane.setVisible(true);
   }

   @FXML
   protected void onSubscribeButton(ActionEvent event)
   {
      ShapeKind subShapeKind = ShapeKind.valueOf(
            subShapeBox.getSelectionModel().getSelectedItem().toUpperCase());

      // add subscription
      addSubscription(subShapeKind,
            filterPaneController.getFilter(),
            subscriberQoSPaneController.getQoS());
   }

}
