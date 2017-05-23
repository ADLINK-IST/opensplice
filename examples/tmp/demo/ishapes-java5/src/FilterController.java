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

import java.net.URL;
import java.util.ResourceBundle;
import java.util.logging.Level;

import javafx.event.ActionEvent;
import javafx.fxml.FXML;
import javafx.fxml.Initializable;
import javafx.scene.control.RadioButton;
import javafx.scene.control.TextArea;
import javafx.scene.layout.AnchorPane;

public class FilterController implements Initializable {

    private static final Logger log = Logger.getInstance();

    @FXML
    private AnchorPane filterPane;

    @FXML
    private RadioButton enableButton;
    @FXML
    private RadioButton disableButton;
    @FXML
    private TextArea jsCodeArea;

    private Filter<ShapeType> filter = null;

    @Override
    public void initialize(URL fxmlFileLocation, ResourceBundle resources) {
        assert filterPane != null : "fx:id=\"filterPane\" was not injected: check your FXML file 'SubscriberQoS.fxml'.";
        assert enableButton != null : "fx:id=\"enableButton\" was not injected: check your FXML file 'SubscriberQoS.fxml'.";
        assert disableButton != null : "fx:id=\"disableButton\" was not injected: check your FXML file 'SubscriberQoS.fxml'.";
        assert jsCodeArea != null : "fx:id=\"jsCodeArea\" was not injected: check your FXML file 'SubscriberQoS.fxml'.";

    }

    public Filter<ShapeType> getFilter() {
        return filter;
    }

    private boolean setFilter() {
        if (jsCodeArea.isDisabled() || jsCodeArea.getText().isEmpty()) {
            filter = null;
        } else {
            filter = new Filter<ShapeType>(jsCodeArea.getText());
        }
        return true;
    }

    @FXML
    protected void onEnableSelected(ActionEvent event) {
        jsCodeArea.setDisable(false);
    }

    @FXML
    protected void onDisableSelected(ActionEvent event) {
        jsCodeArea.setDisable(true);
    }

    @FXML
    protected void onCancel(ActionEvent event) {
        log.log("onCancel", Level.FINE);
        filterPane.setVisible(false);
    }

    @FXML
    protected void onOK(ActionEvent event) {
        log.log("onOK", Level.FINE);

        if (setFilter())
            filterPane.setVisible(false);
    }

}
