/*
 *                         Vortex OpenSplice
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
package org.opensplice.common.view;

import java.util.Date;

import org.opensplice.cmdataadapter.TypeInfo.TypeEvolution;

/**
 * TypeEvolutionCBViewHolder holds TypeEvolution objects and override the
 * toString() method for display purposes in a JComboBox.
 */
public class TypeEvolutionCBViewHolder implements Comparable<TypeEvolutionCBViewHolder> {
    private TypeEvolution typeEvo;

    public static TypeEvolutionCBViewHolder[] createTypeEvoHolders(TypeEvolution[] typeEvos) {
        TypeEvolutionCBViewHolder[] result = new TypeEvolutionCBViewHolder[typeEvos.length];
        for (int i = 0; i < typeEvos.length; i ++) {
            result[i] = new TypeEvolutionCBViewHolder(typeEvos[i]);
        }
        return result;
    }

    public TypeEvolutionCBViewHolder (TypeEvolution typeEvo) {
        this.typeEvo = typeEvo;
    }

    /**
     * @return The TypeEvolution that is held by this object.
     */
    public TypeEvolution getTypeEvolution () {
        return typeEvo;
    }

    /**
     * Display TypeEvolutions as a readable Date representing the
     * registration time of the type evolution.
     *
     * @return The Date string of the type evolution's registration time.
     */
    public String toString () {
        Date d = new Date(typeEvo.getWriteTime().getMilliSecs());
        return d.toString();
    }

    @Override
    public int compareTo(TypeEvolutionCBViewHolder o) {
        return typeEvo.compareTo(o.getTypeEvolution());
    }

    @Override
    public boolean equals(Object o) {
        if (o instanceof TypeEvolutionCBViewHolder) {
            return typeEvo.equals(((TypeEvolutionCBViewHolder) o).getTypeEvolution());
        }
        return false;
    }

    @Override
    public int hashCode() {
        return typeEvo.hashCode();
    }
}
