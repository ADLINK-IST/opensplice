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

import java.awt.Dimension;
import java.awt.GridBagConstraints;
import java.awt.GridBagLayout;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.util.List;

import javax.swing.JComboBox;
import javax.swing.JPanel;
import javax.swing.JTextField;

import org.opensplice.cmdataadapter.CmDataException;
import org.opensplice.cmdataadapter.TypeInfo;
import org.opensplice.cmdataadapter.TypeInfo.TypeEvolution;
import org.opensplice.cmdataadapter.protobuf.ProtobufDataAdapterFactory;
import org.opensplice.common.view.SelectNameValuePanel;
import org.opensplice.common.view.TextNameValuePanel;
import org.opensplice.common.view.TypeEvolutionCBViewHolder;

public class EntityInfoTabView extends JPanel {

    private final Dimension defaultLabelDim = new Dimension(120, 20);
    private final Dimension defaultFieldDim = new Dimension(340, 20);

    private TextNameValuePanel typeNameText;
    private SelectNameValuePanel typeEvolutionsCombo;
    private TextNameValuePanel typeHashText;
    private EntityInfoPane editorPane;

    public EntityInfoTabView (String contentType, TypeInfo typeInfo) {
        if (typeInfo != null) {
            this.setLayout(new GridBagLayout());
            List<TypeEvolution> typeEvos = typeInfo.getAllTypeEvolutions();
            initTypeNameText(typeInfo);
            if (!typeEvos.isEmpty()) {
                initTypeEvolutionsCombo(typeInfo);
                initTypeHashText(typeInfo.getMostRecentEvolution());
            }
            getEditorPaneForType(contentType, typeInfo, typeInfo.getMostRecentEvolution());
        } else {
            getEditorPaneForType(contentType, null, null);
        }
    }

    private TextNameValuePanel initTypeNameText (TypeInfo typeInfo) {
        if (typeNameText == null) {
            typeNameText = new TextNameValuePanel("Type Name", typeInfo.getTypeName(),
                    false, defaultLabelDim, defaultFieldDim);
            ((JTextField)typeNameText.getField()).setEditable(false);
            GridBagConstraints c = new GridBagConstraints();
            c.gridy = 0;
            this.add(typeNameText, c);
        }
        return typeNameText;
    }

    private SelectNameValuePanel initTypeEvolutionsCombo (TypeInfo typeInfo) {
        if (typeEvolutionsCombo == null) {
            TypeEvolutionCBViewHolder[] typeEvoHolders =
                    TypeEvolutionCBViewHolder.createTypeEvoHolders(typeInfo.getAllTypeEvolutions().
                            toArray(new TypeEvolution[0]));
            typeEvolutionsCombo = new SelectNameValuePanel("Type Evolutions", typeEvoHolders,
                    false, defaultLabelDim, defaultFieldDim);
            GridBagConstraints c = new GridBagConstraints();
            c.gridy = 1;
            this.add(typeEvolutionsCombo, c);

            ((JComboBox)typeEvolutionsCombo.getField()).addActionListener(new ActionListener() {
                // When the user changes the selected type evolution
                @Override
                public void actionPerformed(ActionEvent e) {
                    final TypeEvolution selectedTypeEvo = ((TypeEvolutionCBViewHolder) ((JComboBox) e
                            .getSource()).getSelectedItem())
                            .getTypeEvolution();
                            getEditorPaneForType(editorPane.getCurrentContentType(),
                                    selectedTypeEvo.getTypeInfo(), selectedTypeEvo);
                            editorPane.setCaretPosition(0);
                            ((JTextField)typeHashText.getField()).setText(selectedTypeEvo.getTypeHash());
                }
            });
        }
        return typeEvolutionsCombo;
    }

    private TextNameValuePanel initTypeHashText (TypeEvolution typeEvo) {
        if (typeHashText == null) {
            typeHashText = new TextNameValuePanel("Type Hash", "",
                    false, defaultLabelDim, defaultFieldDim);
            ((JTextField)typeHashText.getField()).setEditable(false);
            GridBagConstraints c = new GridBagConstraints();
            c.gridy = 2;
            this.add(typeHashText, c);
        }
        if (typeEvo != null) {
            ((JTextField)typeHashText.getField()).setText(typeEvo.getTypeHash());
        }
        return typeHashText;
    }

    /**
     * Creates a new EntityInfoPane to display the selected data type. If an EntityInfoPane is already present,
     * then it is removed from the parent JPanel and the new one is added.
     */
    private EntityInfoPane getEditorPaneForType (String contentType, TypeInfo typeInfo, TypeEvolution typeEvo) {
        if (editorPane != null) {
            this.remove(editorPane);
        }
        if (typeInfo != null) {
            setEditorPaneContent(contentType, typeInfo, typeEvo);
        } else {
            setEditorPaneNoContent(contentType);
        }
        GridBagConstraints c = new GridBagConstraints();
        c.gridy = 3;
        c.fill = GridBagConstraints.BOTH;
        c.weightx = 1;
        c.weighty = 1;
        this.add(editorPane, c);
        this.repaint();
        this.revalidate();
        return editorPane;
    }

    private void setEditorPaneContent(String contentType, TypeInfo typeInfo, TypeEvolution typeEvo) {
        editorPane = new EntityInfoPane(contentType);
            if (typeEvo != null
                    && typeInfo.getDataRepresentationId() == TypeInfo.GPB_DATA_ID
                    && ProtobufDataAdapterFactory.getInstance().isEnabled()) {
                try {
                    // If rendering a protobuf type, force the editor pane to plain text,
                    // and invoke the .proto format typedef printer.
                    String protobufType = ProtobufDataAdapterFactory.getInstance()
                            .getDescriptorProto(typeEvo);
                    editorPane.setViewType("text/plain");
                    editorPane.setSelection(protobufType);
                } catch (CmDataException cde) {
                    editorPane.setSelection(typeInfo.getBareMetaType());
                }
            } else {
                editorPane.setSelection(typeInfo.getBareMetaType());
            }
        }

    private void setEditorPaneNoContent(String contentType) {
        editorPane = new EntityInfoPane(contentType);
        editorPane.setSelection("Unable to resolve topic type.");
    }

    public EntityInfoPane getEntityInfoPane () {
        return editorPane;
    }
}
