/*
 *                         OpenSplice DDS
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
package org.opensplice.common.view.entity;

import java.awt.Component;
import java.awt.FlowLayout;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;

import javax.swing.BoxLayout;
import javax.swing.JButton;
import javax.swing.JFrame;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.WindowConstants;

import org.opensplice.cm.Entity;
import org.opensplice.cmdataadapter.CmDataException;
import org.opensplice.cmdataadapter.TypeInfo;
import org.opensplice.cmdataadapter.TypeInfo.TypeEvolution;
import org.opensplice.cmdataadapter.protobuf.ProtobufDataAdapterFactory;
import org.opensplice.common.view.StatusPanel;
import org.opensplice.common.view.TypeEvolutionCBViewHolder;

public class DataTypeFrame extends JFrame {

    private TypeEvolutionCBViewHolder typeEvolutionViewHolder;
    private StatusPanel statusPanel;
    private JPanel rootPanel;
    private JPanel buttonPanel;
    private JButton okButton;
    private EntityInfoPane dataTypePane;

    public DataTypeFrame (Entity _entity, TypeEvolutionCBViewHolder _typeEvolutionViewHolder) {
        super((_entity != null ?_entity.toStringExtended() : "") + " | Type Evolution Definiton");
        typeEvolutionViewHolder = _typeEvolutionViewHolder;
        initialize();
    }

    public void setTitle (String entityName) {
        super.setTitle(entityName + " | Type Evolution Definiton");
    }

    public void setTypeEvolution (TypeEvolutionCBViewHolder _typeEvolutionViewHolder) {
        TypeEvolution typeEvo = _typeEvolutionViewHolder.getTypeEvolution();
        if (ProtobufDataAdapterFactory.getInstance().isEnabled()
                && typeEvo.getTypeInfo().getDataRepresentationId() == TypeInfo.GPB_DATA_ID) {
            typeEvolutionViewHolder = _typeEvolutionViewHolder;
            try {
                dataTypePane.setText(ProtobufDataAdapterFactory.getInstance()
                        .getDescriptorProto(typeEvo));
            } catch (CmDataException e) {
                setStatus("Error: " + e.getMessage(), false, false);
            }
        }
        setStatus(/*"Evolution: " + */typeEvolutionViewHolder.toString() + " (" + typeEvo.getTypeHash() + ")", true, false);
        dataTypePane.setCaretPosition(0);
    }

    /**
     * This method initializes this
     *
     * @return void
     */
    private void initialize() {
        this.setSize(630,300);
        this.setDefaultCloseOperation(WindowConstants.DISPOSE_ON_CLOSE);
        this.setContentPane(getJContentPane());
        this.setResizable(true);
    }

    /**
     * This method initializes rootPanel
     *
     * @return The root panel of the frame.
     */
    private JPanel getJContentPane() {
        if(rootPanel == null) {
            rootPanel = new JPanel();
            rootPanel.setLayout(new java.awt.BorderLayout());
            rootPanel.add(getBottomPanel(), java.awt.BorderLayout.SOUTH);
            rootPanel.add(getMainPanel(), java.awt.BorderLayout.CENTER);
        }
        return rootPanel;
    }

    private Component getBottomPanel() {
        JPanel bottomPanel = new JPanel();
        bottomPanel.setLayout(new BoxLayout(bottomPanel, javax.swing.BoxLayout.Y_AXIS));
        bottomPanel.add(getButtonPanel());
        bottomPanel.add(getStatusPanel());
        return bottomPanel;
    }

    private JScrollPane getMainPanel() {
        JScrollPane scrollPane = new JScrollPane();
        dataTypePane = new EntityInfoPane("text/plain");
        setTypeEvolution(typeEvolutionViewHolder);
        scrollPane.setViewportView(dataTypePane);
        return scrollPane;
    }

    /**
     * This method initializes the buttonPanel that contains the OK and Cancel button.
     *
     * @return The created or already existing buttonPanel.
     */
    private JPanel getButtonPanel() {
        if(buttonPanel == null) {
            buttonPanel = new JPanel();
            FlowLayout layFlowLayout = new FlowLayout(FlowLayout.CENTER);
            buttonPanel.setLayout(layFlowLayout);
            buttonPanel.add(getOkButton(), null);
        }
        return buttonPanel;
    }

    /**
     * This method initializes the OK button.
     *
     * @return The created or already existing OK button.
     */
    private JButton getOkButton() {
        if(okButton == null) {
            okButton = new JButton();
            okButton.setText("Ok");
            okButton.setMnemonic('O');
            okButton.addActionListener(new ActionListener() {
                @Override
                public void actionPerformed(ActionEvent e) {
                    dispose();
                }
            });
            okButton.setPreferredSize(new java.awt.Dimension(100, 20));
            this.getRootPane().setDefaultButton(okButton);
        }
        return okButton;
    }

    private StatusPanel getStatusPanel(){
        if(statusPanel == null){
            statusPanel = new StatusPanel(this.getWidth(), "", false, true);
        }
        return statusPanel;
    }

    /**
     * Sets the supplied status in the status bar.
     *
     * @param msg The message to display.
     * @param persistent Whether or not the message should be displayed until
     *                   another message is set, or should be removed after an
     *                   amount of timer
     * @param busy Whether or not the status bar should display the progress bar
     *             and set the progress to indeterminate mode/
     */
    private void setStatus(String msg, boolean persistent, boolean busy){
        statusPanel.setStatus(msg, persistent, busy);
    }
}
