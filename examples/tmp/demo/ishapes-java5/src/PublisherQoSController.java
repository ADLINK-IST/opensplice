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
import java.util.logging.Level;

import org.omg.dds.core.policy.Durability;
import org.omg.dds.core.policy.Ownership;
import org.omg.dds.core.policy.OwnershipStrength;
import org.omg.dds.core.policy.QosPolicy.ForDataWriter;
import org.omg.dds.core.policy.Reliability;
import org.omg.dds.core.policy.TransportPriority;

import javafx.collections.FXCollections;
import javafx.event.ActionEvent;
import javafx.event.EventHandler;
import javafx.fxml.FXML;
import javafx.fxml.Initializable;
import javafx.scene.control.ChoiceBox;
import javafx.scene.control.Label;
import javafx.scene.control.RadioButton;
import javafx.scene.control.TextField;
import javafx.scene.input.KeyEvent;
import javafx.scene.layout.AnchorPane;

public class PublisherQoSController implements Initializable {

    private static final Logger log = Logger.getInstance();

    private static final String DURABILITY_VOLATILE = "Volatile";
    private static final String DURABILITY_TRANSIENT_LOCAL = "TransientLocal";
    private static final String DURABILITY_TRANSIENT = "Transient";
    private static final String DURABILITY_PERSISTENT = "Persistent";

    private static final String DEFAULT_STRENGTH_VALUE = "50";
    private static final String DEFAULT_PRIORITY_VALUE = "50";

    @FXML
    private AnchorPane publisherQoSPane;

    @FXML
    private RadioButton reliableButton;
    @FXML
    private RadioButton bestEffortButton;
    @FXML
    private RadioButton sharedButton;
    @FXML
    private RadioButton exclusiveButton;
    @FXML
    private Label strengthLabel;
    @FXML
    private TextField strengthField;
    @FXML
    private ChoiceBox<String> durabilityChoices;
    @FXML
    private TextField priorityField;

    private ShapesDDSManager shapesDDS;
    private Reliability reliability;
    private Ownership ownership;
    private OwnershipStrength ownershipStrength;
    private Durability durability;
    private TransportPriority transportPriority;

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
        assert bestEffortButton != null : "fx:id=\"bestEffortButton\" was not injected: check your FXML file 'PublisherQoS.fxml'.";
        assert durabilityChoices != null : "fx:id=\"durabilityChoices\" was not injected: check your FXML file 'PublisherQoS.fxml'.";
        assert exclusiveButton != null : "fx:id=\"exclusiveButton\" was not injected: check your FXML file 'PublisherQoS.fxml'.";
        assert priorityField != null : "fx:id=\"priorityField\" was not injected: check your FXML file 'PublisherQoS.fxml'.";
        assert publisherQoSPane != null : "fx:id=\"publisherQoSPane\" was not injected: check your FXML file 'PublisherQoS.fxml'.";
        assert reliableButton != null : "fx:id=\"reliableButton\" was not injected: check your FXML file 'PublisherQoS.fxml'.";
        assert sharedButton != null : "fx:id=\"sharedButton\" was not injected: check your FXML file 'PublisherQoS.fxml'.";
        assert strengthLabel != null : "fx:id=\"strengthLabel\" was not injected: check your FXML file 'PublisherQoS.fxml'.";
        assert strengthField != null : "fx:id=\"strengthField\" was not injected: check your FXML file 'PublisherQoS.fxml'.";

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
        strengthField.setText(DEFAULT_STRENGTH_VALUE);
        priorityField.setText(DEFAULT_PRIORITY_VALUE);

        // filter-out non-number key pressed
        strengthField.addEventFilter(KeyEvent.KEY_TYPED, numberFilter);
        strengthField.addEventFilter(KeyEvent.KEY_TYPED, numberFilter);
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

        // OwnershipStrength QoS
        try {
            ownershipStrength = shapesDDS.getPolicyFactory()
                    .OwnershipStrength()
                    .withValue(Integer.valueOf(strengthField.getText()));
        } catch (NumberFormatException e) {
            log.log("Internal error: Publisher OwnershipStrength is not a number: "
                    + strengthField.getText(), Level.SEVERE);
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

        // TransportPriority QoS
        try {
            transportPriority = shapesDDS.getPolicyFactory()
                    .TransportPriority()
                    .withValue(Integer.valueOf(priorityField.getText()));
        } catch (NumberFormatException e) {
            log.log("Internal error: Publisher OwnershipStrength is not a number: "
                    + priorityField.getText(), Level.SEVERE);
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
            strengthLabel.setDisable(true);
            strengthField.setDisable(true);
        } else {
            exclusiveButton.setSelected(true);
            strengthLabel.setDisable(false);
            strengthField.setDisable(false);
        }

        // OwnershipStrength QoS
        strengthField.setText("" + ownershipStrength.getValue());

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

        // TransportPriority QoS
        priorityField.setText("" + transportPriority.getValue());

    }

    public ForDataWriter[] getQoS() {
        ForDataWriter[] qos = new ForDataWriter[5];
        qos[0] = reliability;
        qos[1] = ownership;
        qos[2] = ownershipStrength;
        qos[3] = durability;
        qos[4] = transportPriority;
        return qos;
    }

    @FXML
    protected void onSharedSelected(ActionEvent event) {
        strengthLabel.setDisable(true);
        strengthField.setDisable(true);
    }

    @FXML
    protected void onExclusiveSelected(ActionEvent event) {
        strengthLabel.setDisable(false);
        strengthField.setDisable(false);
    }

    @FXML
    protected void onStrengthChange(ActionEvent event) {
        log.log("onStrengthChange", Level.INFO);
        // check if new text is a number
        if (!isValidNumber(strengthField.getText())) {
            strengthField.setText(DEFAULT_STRENGTH_VALUE);
        }
    }

    @FXML
    protected void onPriorityChange(ActionEvent event) {
        log.log("onPriorityChange", Level.INFO);
        if (!isValidNumber(priorityField.getText())) {
            priorityField.setText(DEFAULT_PRIORITY_VALUE);
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
        publisherQoSPane.setVisible(false);
    }

    @FXML
    protected void onOK(ActionEvent event) {
        log.log("onOK", Level.INFO);
        saveQoS();
        publisherQoSPane.setVisible(false);
    }

}
