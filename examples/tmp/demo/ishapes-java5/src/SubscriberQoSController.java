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
import java.util.ResourceBundle;
import java.util.concurrent.TimeUnit;
import java.util.logging.Level;

import org.omg.dds.core.policy.Durability;
import org.omg.dds.core.policy.History;
import org.omg.dds.core.policy.Ownership;
import org.omg.dds.core.policy.QosPolicy.ForDataReader;
import org.omg.dds.core.policy.Reliability;
import org.omg.dds.core.policy.TimeBasedFilter;

import javafx.collections.FXCollections;
import javafx.event.ActionEvent;
import javafx.event.EventHandler;
import javafx.fxml.FXML;
import javafx.fxml.Initializable;
import javafx.scene.control.CheckBox;
import javafx.scene.control.ChoiceBox;
import javafx.scene.control.Label;
import javafx.scene.control.RadioButton;
import javafx.scene.control.TextField;
import javafx.scene.input.KeyEvent;
import javafx.scene.layout.AnchorPane;

public class SubscriberQoSController implements Initializable {

    private static final Logger log = Logger.getInstance();

    private static final String DURABILITY_VOLATILE = "Volatile";
    private static final String DURABILITY_TRANSIENT_LOCAL = "TransientLocal";
    private static final String DURABILITY_TRANSIENT = "Transient";
    private static final String DURABILITY_PERSISTENT = "Persistent";

    private static final String DEFAULT_DEPTH_VALUE = "1";
    private static final String DEFAULT_PERIOD_VALUE = "500";

    @FXML
    private AnchorPane subscriberQoSPane;

    @FXML
    private RadioButton reliableButton;
    @FXML
    private RadioButton bestEffortButton;
    @FXML
    private RadioButton sharedButton;
    @FXML
    private RadioButton exclusiveButton;
    @FXML
    private RadioButton keepAllButton;
    @FXML
    private RadioButton keepLastButton;
    @FXML
    private Label depthLabel;
    @FXML
    private TextField depthField;
    @FXML
    private ChoiceBox<String> durabilityChoices;
    @FXML
    private Label periodLablel;
    @FXML
    private TextField periodField;
    @FXML
    private CheckBox timeFilterOnBox;

    private ShapesDDSManager shapesDDS;
    private Reliability reliability;
    private Ownership ownership;
    private History history;
    private Durability durability;
    private TimeBasedFilter timeBasedFilter;

    // Key event filter disabling non-number key typed
    private EventHandler<KeyEvent> numberFilter = new EventHandler<KeyEvent>() {

        @Override
        public void handle(KeyEvent event) {
            if (!"0123456789".contains(event.getCharacter())) {
                event.consume();
            }
        }
    };

    @Override
    public void initialize(URL fxmlFileLocation, ResourceBundle resources) {
        assert bestEffortButton != null : "fx:id=\"bestEffortButton\" was not injected: check your FXML file 'SubscriberQoS.fxml'.";
        assert depthLabel != null : "fx:id=\"depthLabel\" was not injected: check your FXML file 'SubscriberQoS.fxml'.";
        assert depthField != null : "fx:id=\"depthField\" was not injected: check your FXML file 'SubscriberQoS.fxml'.";
        assert durabilityChoices != null : "fx:id=\"durabilityChoices\" was not injected: check your FXML file 'SubscriberQoS.fxml'.";
        assert exclusiveButton != null : "fx:id=\"exclusiveButton\" was not injected: check your FXML file 'SubscriberQoS.fxml'.";
        assert keepAllButton != null : "fx:id=\"keepAllButton\" was not injected: check your FXML file 'SubscriberQoS.fxml'.";
        assert keepLastButton != null : "fx:id=\"keepLastButton\" was not injected: check your FXML file 'SubscriberQoS.fxml'.";
        assert periodField != null : "fx:id=\"periodField\" was not injected: check your FXML file 'SubscriberQoS.fxml'.";
        assert reliableButton != null : "fx:id=\"reliableButton\" was not injected: check your FXML file 'SubscriberQoS.fxml'.";
        assert sharedButton != null : "fx:id=\"sharedButton\" was not injected: check your FXML file 'SubscriberQoS.fxml'.";
        assert subscriberQoSPane != null : "fx:id=\"subscriberQoSPane\" was not injected: check your FXML file 'SubscriberQoS.fxml'.";
        assert timeFilterOnBox != null : "fx:id=\"timeFilterOnBox\" was not injected: check your FXML file 'SubscriberQoS.fxml'.";

        // set durabilityChoices choices
        durabilityChoices.setItems(FXCollections.observableArrayList(
                DURABILITY_VOLATILE, DURABILITY_TRANSIENT_LOCAL/*
                                                                * ,
                                                                * DURABILITY_TRANSIENT
                                                                * ,
                                                                * DURABILITY_PERSISTENT
                                                                */));
        durabilityChoices.getSelectionModel().select(0);

        // set default fields values
        depthField.setText(DEFAULT_DEPTH_VALUE);
        periodField.setText(DEFAULT_PERIOD_VALUE);

        // filter-out non-number key pressed
        depthField.addEventFilter(KeyEvent.KEY_TYPED, numberFilter);
        periodField.addEventFilter(KeyEvent.KEY_TYPED, numberFilter);
    }

    void setShapesDDSManager(ShapesDDSManager shapesDDS) {
        this.shapesDDS = shapesDDS;
    }

    void saveQoS() {
        // Reliability QoS
        if (reliableButton.isSelected()) {
            reliability = shapesDDS.getPolicyFactory().Reliability()
                    .withReliable();
        } else {
            reliability = shapesDDS.getPolicyFactory().Reliability()
                    .withBestEffort();
        }

        // Ownership QoS
        if (sharedButton.isSelected()) {
            ownership = shapesDDS.getPolicyFactory().Ownership().withShared();
        } else {
            ownership = shapesDDS.getPolicyFactory().Ownership()
                    .withExclusive();
        }

        // History QoS
        if (keepAllButton.isSelected()) {
            history = shapesDDS.getPolicyFactory().History().withKeepAll();
        } else {
            try {
                history = shapesDDS.getPolicyFactory().History()
                        .withKeepLast(Integer.valueOf(depthField.getText()));
            } catch (NumberFormatException e) {
                log.log("Internal error: Subscriber History QoS Depth is not a number: "
                        + depthField.getText(), Level.SEVERE);
            }
        }

        // Durability QoS
        String choice = durabilityChoices.getSelectionModel().getSelectedItem();
        if (choice == DURABILITY_VOLATILE) {
            durability = shapesDDS.getPolicyFactory().Durability()
                    .withVolatile();
        } else if (choice == DURABILITY_TRANSIENT_LOCAL) {
            durability = shapesDDS.getPolicyFactory().Durability()
                    .withTransientLocal();
        } else if (choice == DURABILITY_TRANSIENT) {
            durability = shapesDDS.getPolicyFactory().Durability()
                    .withTransient();
        } else if (choice == DURABILITY_PERSISTENT) {
            durability = shapesDDS.getPolicyFactory().Durability()
                    .withPersistent();
        }

        // TimeBasedFilter QoS
        if (timeFilterOnBox.isSelected()) {
            try {
                timeBasedFilter = shapesDDS
                        .getPolicyFactory()
                        .TimeBasedFilter()
                        .withMinimumSeparation(
                                Integer.valueOf(periodField.getText()),
                                TimeUnit.MILLISECONDS);
            } catch (NumberFormatException e) {
                log.log("Internal error: Subscriber TimeBasedFilter period is not a number: "
                        + periodField.getText(), Level.SEVERE);
            }
        } else {
            timeBasedFilter = null;
        }
    }

    private void resetQoS() {
        // Reliability QoS
        if (reliability.getKind() == Reliability.Kind.RELIABLE) {
            reliableButton.setSelected(true);
        } else {
            bestEffortButton.setSelected(true);
        }

        // Ownership QoS
        if (ownership.getKind() == Ownership.Kind.SHARED) {
            sharedButton.setSelected(true);
        } else {
            exclusiveButton.setSelected(true);
        }

        // History QoS
        if (history.getKind() == History.Kind.KEEP_ALL) {
            keepAllButton.setSelected(true);
            depthLabel.setDisable(true);
            depthField.setDisable(true);
        } else {
            keepLastButton.setSelected(true);
            depthLabel.setDisable(false);
            depthField.setDisable(false);
            depthField.setText("" + history.getDepth());
        }

        // Durability QoS
        switch (durability.getKind()) {
        case VOLATILE:
            durabilityChoices.getSelectionModel().select(DURABILITY_VOLATILE);
            break;
        case TRANSIENT_LOCAL:
            durabilityChoices.getSelectionModel().select(
                    DURABILITY_TRANSIENT_LOCAL);
            break;
        case TRANSIENT:
            durabilityChoices.getSelectionModel().select(DURABILITY_TRANSIENT);
            break;
        case PERSISTENT:
            durabilityChoices.getSelectionModel().select(DURABILITY_PERSISTENT);
            break;
        }

        // TimeBasedFilter QoS
        if (timeBasedFilter != null) {
            timeFilterOnBox.setSelected(true);
            periodLablel.setDisable(false);
            periodField.setDisable(false);
            periodField.setText(""
                    + timeBasedFilter.getMinimumSeparation().getDuration(
                            TimeUnit.MILLISECONDS));
        } else {
            timeFilterOnBox.setSelected(false);
            periodLablel.setDisable(true);
            periodField.setDisable(true);
        }
    }

    public ForDataReader[] getQoS() {
        ForDataReader[] qos = new ForDataReader[timeBasedFilter != null ? 5 : 4];
        qos[0] = reliability;
        qos[1] = ownership;
        qos[2] = history;
        qos[3] = durability;
        if (timeBasedFilter != null)
            qos[4] = timeBasedFilter;
        return qos;
    }

    @FXML
    protected void onKeepAllSelected(ActionEvent event) {
        depthLabel.setDisable(true);
        depthField.setDisable(true);
    }

    @FXML
    protected void onKeepLastSelected(ActionEvent event) {
        depthLabel.setDisable(false);
        depthField.setDisable(false);
    }

    @FXML
    protected void onDepthChange(ActionEvent event) {
        log.log("onDepthChange", Level.INFO);
        // check if new text is a number
        if (!isValidNumber(depthField.getText())) {
            depthField.setText(DEFAULT_DEPTH_VALUE);
        }
    }

    @FXML
    protected void onPeriodChange(ActionEvent event) {
        log.log("onPriorityChange", Level.INFO);

        if (!isValidNumber(periodField.getText())) {
            periodField.setText(DEFAULT_PERIOD_VALUE);
        }
    }

    @FXML
    protected void onTimeFilterAction(ActionEvent event) {
        log.log("onTimeFilterAction", Level.INFO);

        if (timeFilterOnBox.isSelected()) {
            periodLablel.setDisable(false);
            periodField.setDisable(false);
        } else {
            periodLablel.setDisable(true);
            periodField.setDisable(true);
        }
    }

    private boolean isValidNumber(String text) {
        try {
            Integer.parseInt(text);
            return true;
        } catch (NumberFormatException e) {
            return false;
        }
    }

    @FXML
    protected void onCancel(ActionEvent event) {
        log.log("onCancel", Level.INFO);
        resetQoS();
        subscriberQoSPane.setVisible(false);
    }

    @FXML
    protected void onOK(ActionEvent event) {
        log.log("onOK", Level.INFO);
        saveQoS();
        subscriberQoSPane.setVisible(false);
    }

}
