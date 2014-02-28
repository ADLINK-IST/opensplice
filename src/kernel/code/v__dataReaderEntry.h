/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2013 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE
 *
 *   for full copyright notice and license terms.
 *
 */

#ifndef V__DATAREADERENTRY_H
#define V__DATAREADERENTRY_H

#include "v_dataReaderEntry.h"

/**
 * \brief Set the specified flags in the instanceState of all DataReader
 * instances associated with the specified DataReaderEntry.
 *
 * \param _this The DataReaderEntry for which all instanceStates must be updated.
 * \param flags The flags that must be set for all instanceStates associated with
 *              _this.
 */
void
v_dataReaderEntryMarkInstanceStates (
    v_dataReaderEntry _this,
    c_ulong flags);

/**
 * \brief Reset the specified flags in the instanceState of all DataReader
 *  instances associated with the specified DataReaderEntry.
 *
 * \param _this The DataReaderEntry for which all instanceStates must be updated.
 * \param flags The flags that must be reset for all instanceStates associated with
 *              _this.
 */
void
v_dataReaderEntryUnmarkInstanceStates (
    v_dataReaderEntry _this,
    c_ulong flags);

#endif
