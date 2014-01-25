/*
 * Copyright 2014 John Pritchard, Syntelos.  All rights reserved.
 */
#ifndef _CONSOLE_MEMMULTIPLEX_MemMultiplexTable_H
#define _CONSOLE_MEMMULTIPLEX_MemMultiplexTable_H

#include <QObject>
#include <QList>

#include "SystemDeviceIdentifier.h"
#include "SystemMultiplexTable.h"

/*!
 * 
 */
class MemMultiplexTable : public SystemMultiplexTable, QList<SystemMessage*> {

 public:
    MemMultiplexTable(const SystemDeviceIdentifier& id, QObject* parent = 0);
    ~MemMultiplexTable();
    /*!
     */
    virtual quint32 getOverheadTemporal();
    /*!
     */
    virtual void setOverheadTemporal(quint32);
    /*!
     */
    virtual quint32 getOverheadSpatial();
    /*!
     */
    virtual void setOverheadSpatial(quint32);
    /*!
     */
    virtual quint32 getRecordCount();
    /*!
     * The external number of records in the application domain,
     * independent of overhead requirements.
     */
    virtual void setRecordCount(quint32);
    /*!
     */
    virtual SystemMessage* query(const SystemMessage& m);
    /*!
     * If the returned value is not a "null qvariant", then the const
     * may be lowered.
     */
    virtual QVariant query(const SystemName&);
    /*!
     * Create a new record.  If the cache is not open, the operation
     * of the multiplex will populate the index.
     */
    virtual void update(const SystemMessage& m);
    /*!
     */
    virtual void update(const SystemNameValue& nvp);
    /*!
     */
    virtual void select(SystemMultiplexSelect& select);

};
#endif
