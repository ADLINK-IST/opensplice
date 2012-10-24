/*
 * in_ddsiElements.c
 *
 *  Created on: Sep 23, 2008
 *      Author: frehberg
 */

#include "in_ddsiElements.h"
#include <assert.h>
#include "in_address.h"
#include "in_ddsiDefinitions.h"
#include <string.h> /* memcmp */
#include "os_heap.h"
#include "os_stdlib.h" /* for strcasecmp */
#include "c_typebase.h"
#include "in_report.h"

#define IN_NTP_FRAC	4294967296.		        /* a billion */
#define IN_EPOCH_JAN_1970 2208988800UL		/* Unix base epoch */
#define IN_MAX(a,b) (((a)>(b))?(a):(b))

/* -------------------------------------------------- */
/* ---- in_ddsiGuidprefix --------------------------- */
/* -------------------------------------------------- */

void in_ddsiGuidPrefixInit(in_ddsiGuidPrefixRef self, const v_gid *gid) {
    assert(sizeof(in_ddsiGuidPrefix) == sizeof(v_gid));
    memcpy(self, gid, sizeof(v_gid));
}

void in_ddsiGuidPrefixDeinit(in_ddsiGuidPrefixRef self) {
    /* nop */
}

/** \return -1 on error, otherwise number of octets written */
in_long
in_ddsiGuidPrefixSerialize(
        in_ddsiGuidPrefixRef self,
        in_ddsiSerializer serializer)
{
    const in_long nofOctets =
        sizeof(in_ddsiGuidPrefix);

    in_long result =
        in_ddsiSerializerAppendOctets(
            serializer,
            self,
            nofOctets);

    assert(nofOctets==12);

    return result;
}

in_long in_ddsiGuidPrefixInitFromBuffer(in_ddsiGuidPrefixRef self,
        in_ddsiDeserializer reader) {
    in_long result = 0;

    assert(in_ddsiDeserializerNofUnreadOctets(reader) >= (in_long) sizeof(in_ddsiGuidPrefix));

    result += in_ddsiDeserializerParseOctets(reader, (in_octet *) self,
            sizeof(in_ddsiGuidPrefix));

    return result;
}

os_boolean in_ddsiGuidPrefixEqual(
        const in_ddsiGuidPrefixRef self,
        const in_ddsiGuidPrefixRef other)
{
    assert(sizeof(in_ddsiGuidPrefix) == 12);

    if (self == other)
        return TRUE;
    if (!self)
        return FALSE;
    if (!other)
        return FALSE;

    return memcmp(self, other, sizeof(in_ddsiGuidPrefix)) == 0;
}

/* -------------------------------------------------- */
/* ---- in_ddsiBuiltinEndpoint ---------------------- */
/* -------------------------------------------------- */

in_long in_ddsiBuiltinEndpointSetSerialize(in_ddsiBuiltinEndpointSet self,
        in_ddsiSerializer writer) {
    assert(self);
    assert(writer);

    return in_ddsiSerializerAppendUlong(writer, self->flags);
}

in_long in_ddsiBuiltinEndpointSetInitFromBuffer(in_ddsiBuiltinEndpointSet self,
        in_ddsiDeserializer reader) {
    return in_ddsiDeserializerParseUlong(reader, &(self->flags));
}

os_boolean in_ddsiBuiltinEndpointSetEqual(in_ddsiBuiltinEndpointSet self,
        in_ddsiBuiltinEndpointSet other) {
    if (self == other)
        return TRUE;
    if (self == NULL)
        return FALSE;
    if (other == NULL)
        return FALSE;

    return self->flags == other->flags;
}

/* -------------------------------------------------- */
/* ---- in_ddsiTopicKind ---------------------------- */
/* -------------------------------------------------- */

in_long in_ddsiTopicKindSerialize(in_ddsiTopicKind self,
        in_ddsiSerializer writer) {
    assert(self);
    assert(writer);

    return in_ddsiSerializerAppendLong(writer, self->value);
}

in_long in_ddsiTopicKindInitFromBuffer(in_ddsiTopicKind self,
        in_ddsiDeserializer reader) {
    return in_ddsiDeserializerParseLong(reader, &(self->value));
}

os_boolean in_ddsiTopicKindEqual(in_ddsiTopicKind self, in_ddsiTopicKind other) {
    if (self == other)
        return TRUE;
    if (self == NULL)
        return FALSE;
    if (other == NULL)
        return FALSE;

    return self->value == other->value;
}

/* -------------------------------------------------- */
/* ---- in_ddsiStatusInfo --------------------------- */
/* -------------------------------------------------- */

in_long in_ddsiStatusInfoSerialize(in_ddsiStatusInfo self,
        in_ddsiSerializer writer) {
    assert(self);
    assert(writer);

    return in_ddsiSerializerAppendLong(writer, self->value);
}

in_long in_ddsiStatusInfoInitFromBuffer(in_ddsiStatusInfo self,
        in_ddsiDeserializer reader) {
    return in_ddsiDeserializerParseLong(reader, &(self->value));
}

os_boolean in_ddsiStatusInfoEqual(in_ddsiStatusInfo self,
        in_ddsiStatusInfo other) {
    if (self == other)
        return TRUE;
    if (self == NULL)
        return FALSE;
    if (other == NULL)
        return FALSE;

    return self->value == other->value;
}

/* -------------------------------------------------- */
/* ---- in_ddsiProductVersion ----------------------- */
/* -------------------------------------------------- */

in_long in_ddsiProductVersionSerialize(in_ddsiProductVersion self,
        in_ddsiSerializer writer) {
    assert(self);
    assert(writer);

    return in_ddsiSerializerAppendOctets(writer, (in_octet *) self->version,
            sizeof(self->version));
}

in_long in_ddsiProductVersionInitFromBuffer(in_ddsiProductVersion self,
        in_ddsiDeserializer reader) {
    assert(sizeof(self->version) == 4);

    return in_ddsiDeserializerParseOctets(reader, (in_octet*) self->version,
            sizeof(self->version));
}

os_boolean in_ddsiProductVersionEqual(in_ddsiProductVersion self,
        in_ddsiProductVersion other) {
    if (self == other)
        return TRUE;
    if (self == NULL)
        return FALSE;
    if (other == NULL)
        return FALSE;
    return memcmp(self->version, other->version, sizeof(self->version)) == 0;
}

/* -------------------------------------------------- */
/* ---- in_ddsiEntityId ----------------------------- */
/* -------------------------------------------------- */

in_long in_ddsiEntityIdSerialize(in_ddsiEntityId self, in_ddsiSerializer writer) {
    assert(sizeof(*self)==4);
    return in_ddsiSerializerAppendOctets(writer, (in_octet*) self,
            sizeof(*self));
}

in_long in_ddsiEntityIdInitFromBuffer(in_ddsiEntityId self,
        in_ddsiDeserializer reader) {
    in_long result = 0;

    assert(sizeof(*self) == 4);
    assert(in_ddsiDeserializerNofUnreadOctets(reader) >= (in_long) sizeof(*self));

    result += in_ddsiDeserializerParseOctets(reader, (in_octet*) self,
            sizeof(*self));

    return result;
}

os_boolean in_ddsiEntityIdEqual(in_ddsiEntityId self, in_ddsiEntityId other) {
    if (self == other)
        return TRUE;
    if (!self)
        return FALSE;
    if (!other)
        return FALSE;

    return self->entityKind == other->entityKind && memcmp(self->entityKey,
            other->entityKey, sizeof(self->entityKey)) == 0;
}

os_uint32
in_ddsiEntityIdAsUint32(in_ddsiEntityId self)
{
    os_uint32 result =
        (self->entityKey[0] << 24) +
        (self->entityKey[1] << 16) +
        (self->entityKey[2] << 8) +
        (self->entityKind);

    return result;
}
/* -------------------------------------------------- */
/* ---- in_ddsiGuid --------------------------------- */
/* -------------------------------------------------- */


in_long
in_ddsiGuidSerialize(
    in_ddsiGuid self,
    in_ddsiSerializer writer)
{
    in_long result;

    assert(self);
    assert(writer);

    result = in_ddsiSerializerAppendOctets(writer, (in_octet*) self,
            sizeof(*self));



    return result;
}

os_boolean
in_ddsiGuidInit(
        in_ddsiGuid _this,
        in_ddsiGuidPrefixRef guidPrefix,
        in_ddsiEntityId entityId)
{
    _this->entityId = *entityId;
    memcpy(_this->guidPrefix, guidPrefix, sizeof(in_ddsiGuidPrefix));

    return OS_TRUE;
}

in_long
in_ddsiGuidInitFromBuffer(in_ddsiGuid self, in_ddsiDeserializer reader) {
    in_long result;
    in_long tmp;
    assert(in_ddsiDeserializerNofUnreadOctets(reader)>=16);

    result = in_ddsiGuidPrefixInitFromBuffer(self->guidPrefix, reader);
    if (result < 0) {
        /* nop */
    } else {
        /* else branch is fast path */
        tmp = in_ddsiEntityIdInitFromBuffer(&(self->entityId), reader);
        if (tmp < 0) {
            result = -1;
        } else {
            result += tmp;
        }
    }
    return result;
}

os_boolean in_ddsiGuidEqual(in_ddsiGuid self, in_ddsiGuid other) {
    assert(sizeof(*self) == 16);

    if (self == other)
        return TRUE;
    if (!self)
        return FALSE;
    if (!other)
        return FALSE;

    return in_ddsiGuidPrefixEqual(self->guidPrefix, other->guidPrefix)
            && in_ddsiEntityIdEqual(&(self->entityId), &(other->entityId));
}

/* -------------------------------------------------- */
/* ---- in_ddsiTime --------------------------------- */
/* -------------------------------------------------- */

const OS_STRUCT (in_ddsiTime) IN_DDSI_TIME_ZERO = {0, 0};
const OS_STRUCT (in_ddsiTime) IN_DDSI_TIME_INVALID = {-1, 0xffffffff};
const OS_STRUCT (in_ddsiTime) IN_DDSI_TIME_INFINITE = {0x7fffffff, 0xffffffff};
const OS_STRUCT (in_ddsiTime) IN_DDSI_TIME_MIN_INFINITE = {0x80000000, 0xffffffff};

void in_ddsiTimeInit(in_ddsiTime self, const c_time *ctime, os_boolean duration) {
    os_double dtemp = 0.0;

    assert(sizeof(self->seconds) == sizeof(ctime->seconds));
    assert(ctime != NULL);
    assert(self != NULL);

    if (ctime->seconds == C_TIME_INVALID.seconds && ctime->nanoseconds
            == C_TIME_INVALID.nanoseconds) {
        *self = IN_DDSI_TIME_INVALID;
    } else if (ctime->seconds == C_TIME_INFINITE.seconds && ctime->nanoseconds
            == C_TIME_INFINITE.nanoseconds) {
        *self = IN_DDSI_TIME_INFINITE;
    } else if (ctime->seconds == C_TIME_MIN_INFINITE.seconds
            && ctime->nanoseconds == C_TIME_MIN_INFINITE.nanoseconds) {
        *self = IN_DDSI_TIME_MIN_INFINITE;
    } else {
        dtemp = (double) ctime->nanoseconds / (double) 1e9;
        if (duration) {
            self->seconds = ((os_uint32) (ctime->seconds));
        } else {
            self->seconds = ((os_uint32) (ctime->seconds)) ;
        }
        self->fraction = dtemp * IN_NTP_FRAC;

        assert(self->fraction> 0 || ctime->nanoseconds == 0);
    }
}

c_equality
in_ddsiTimeCompare(
        const in_ddsiTime t1,
        const in_ddsiTime t2)
{
    if (t1->seconds> t2->seconds) return C_GT;
    if (t1->seconds < t2->seconds) return C_LT;
    if (t1->fraction> t2->fraction) return C_GT;
    if (t1->fraction < t2->fraction ) return C_LT;
    return C_EQ;
}

os_boolean in_ctimeEqualWithTolerance(c_time *t1, c_time *t2) {
    c_time diff;
    c_time tolerance = { 0L, 2L };

    if (t1 == t2)
        return TRUE;
    if (!t1)
        return FALSE;
    if (!t2)
        return FALSE;

    /*creating difference ensure diff>0 */
    if (c_timeCompare(*t1, *t2) == C_LT) {
        diff = c_timeSub(*t2, *t1);
    } else {
        diff = c_timeSub(*t2, *t1);
    }

    if (c_timeCompare(diff, tolerance) == C_LT) {
        return TRUE;
    }

    return FALSE;
}

void in_ddsiTimeAsCTime(in_ddsiTime self, c_time *ctime, os_boolean duration) {
    os_double dtemp = 0.0;

    assert(sizeof(self->seconds) == sizeof(ctime->seconds));
    assert(ctime != NULL);
    assert(self != NULL);

    if (self->seconds == IN_DDSI_TIME_INVALID.seconds && self->fraction
            == IN_DDSI_TIME_INVALID.fraction) {
        *ctime = C_TIME_INVALID;
    } else if (self->seconds == IN_DDSI_TIME_INFINITE.seconds && self->fraction
            == IN_DDSI_TIME_INFINITE.fraction) {
        *ctime = C_TIME_INFINITE;
    } else if (self->seconds == IN_DDSI_TIME_MIN_INFINITE.seconds
            && self->fraction == IN_DDSI_TIME_MIN_INFINITE.fraction) {
        *ctime = C_TIME_MIN_INFINITE;
    } else {
        c_time result = C_TIME_ZERO;

        dtemp = self->fraction / IN_NTP_FRAC;
        /* DDSI declares in Time_t the seconds as signed long, with the effect that
         * current timestamps will be interpreted as negative seconds. Therefore
         * during conversion from NTP timestamp to Unix timestamp the seconds must be
         * casted explicitly as unsigned long (os_uint32) */
        if (duration) {
            result.seconds = ((os_uint32) (self->seconds));
        } else {
        	/*TODO: Check how to represent time - IN_EPOCH_JAN_1970;*/
            result.seconds = ((os_uint32) (self->seconds));
        }

        result.nanoseconds = dtemp * (double) 1e9;

        *ctime = c_timeNormalize(result);
    }
    assert(ctime->seconds >= 0);
    assert(ctime->nanoseconds> 0 || self->fraction == 0);
    assert((ctime->nanoseconds < 1000000000)
            || (ctime->nanoseconds == C_TIME_INFINITE.nanoseconds)
            || (ctime->nanoseconds == C_TIME_INVALID.nanoseconds));
}

in_long in_ddsiTimeSerialize(in_ddsiTime self, in_ddsiSerializer writer) {
    in_long result;
    in_long tmp;

    result = in_ddsiSerializerAppendLong(writer, self->seconds);
    if (result < 0) {
        result = -1;
    } else {
        tmp = in_ddsiSerializerAppendUlong(writer, self->fraction);
        if (tmp < 0) {
            result = -1;
        } else {
            result += tmp;
        }
    }
    return result;
}

in_long in_ddsiTimeInitFromBuffer(in_ddsiTime self, in_ddsiDeserializer reader) {
    in_long result;
    in_long tmp;

    assert(in_ddsiDeserializerNofUnreadOctets(reader) >=
            (in_long) (sizeof(self->seconds) + sizeof(self->fraction)));

    result = in_ddsiDeserializerParseLong(reader, &(self->seconds));
    if (result < 0) {
        result = -1;
    } else {
        tmp = in_ddsiDeserializerParseUlong(reader, &(self->fraction));
        if (tmp < 0) {
            result = -1;
        } else {
            result += tmp;
        }
    }
    return result;
}

in_long
in_ddsiTimeInstantCTimeSerialize(
        const c_time *cTime,
        in_ddsiSerializer cdrWriter,
        os_boolean isDuration)
{
    OS_STRUCT(in_ddsiTime) ddsiTime;
    in_ddsiTimeInit(&ddsiTime, cTime, isDuration);
    return in_ddsiTimeSerialize(&ddsiTime, cdrWriter);
}

in_long in_ddsiTimeInstantCTimeDeserialize(c_time *parsedTime,
        in_ddsiDeserializer cdrReader, os_boolean isDuration) {
    in_long result = 0;
    OS_STRUCT(in_ddsiTime) ddsiTime;
    result = in_ddsiTimeInitFromBuffer(&ddsiTime, cdrReader);
    in_ddsiTimeAsCTime(&ddsiTime, parsedTime, isDuration);
    return result;
}

void in_ddsiTimeDeinit(in_ddsiTime self) {
    /* nop */
}

/* -------------------------------------------------- */
/* ---- in_ddsiVendor* ------------------------------ */
/* -------------------------------------------------- */

in_long in_ddsiVendorSerialize(in_ddsiVendor self, in_ddsiSerializer writer) {
    assert(self);
    assert(writer);
    assert(sizeof(self->vendorId) == 2);

    return in_ddsiSerializerAppendOctets(writer, self->vendorId,
            sizeof(self->vendorId));
}

in_long in_ddsiVendorInitFromBuffer(in_ddsiVendor self,
        in_ddsiDeserializer reader) {
    assert(self);
    assert(reader);
    assert(sizeof(self->vendorId) == 2);
    assert(in_ddsiDeserializerNofUnreadOctets(reader) >= (in_long) sizeof(self->vendorId));

    /* octet streams are not aligned */
    return in_ddsiDeserializerParseOctets(reader, self->vendorId,
            sizeof(self->vendorId));
}

os_boolean in_ddsiVendorEqual(in_ddsiVendor self, in_ddsiVendor other) {
    assert(sizeof(self->vendorId) == 2);

    if (self == other)
        return TRUE;
    if (!self)
        return FALSE;
    if (!other)
        return FALSE;

    return (self->vendorId[0] == other->vendorId[0]) && (self->vendorId[1]
            == other->vendorId[1]);
}

os_ushort in_ddsiVendorToInteger(in_ddsiVendor self) {
    assert(sizeof(os_ushort) == 2); /* 16 bit integer */
    assert(self);
    assert(sizeof(self->vendorId) == 2);

    return (self->vendorId[0] << 8) + (self->vendorId[1]);
}

/* -------------------------------------------------- */
/* ---- in_ddsiSequenceNumber ----------------------- */
/* -------------------------------------------------- */

/** */
static in_int64
in_ddsiSequenceNumberToInt64(in_ddsiSequenceNumber _this)
{
    in_int64 result = _this->high;
    result <<= (8*sizeof(os_int32)); /* shift 32 bit */
    result |= _this->low;

    return result;
}

/** */
void
in_ddsiSequenceNumberInitNative(in_ddsiSequenceNumber _this,
        c_ulong sequenceNumber)
{
    /* TODO Warning: OSPL sequence number covers only the low-bytes */
    assert(sizeof(sequenceNumber) == sizeof(os_uint32));

    in_ddsiSequenceNumberInit(_this, 0, sequenceNumber);
}


/** */
void
in_ddsiSequenceNumberInit(in_ddsiSequenceNumber _this,
        os_int32 high,
        os_uint32 low)
{
    _this->high = high;
    _this->low = low;
}

in_long
in_ddsiSequenceNumberSerialize(in_ddsiSequenceNumber self,
        in_ddsiSerializer writer) {
    in_long result, tmp;

    assert(sizeof(*self)==8);
    assert(sizeof(self->high)==4);
    assert(sizeof(self->low)==4);
    result = in_ddsiSerializerAppendLong(writer, self->high);
    if (result < 0) {
        result = -1;
    } else {
        tmp = in_ddsiSerializerAppendUlong(writer, self->low);
        if (tmp < 0) {
            result = -1;
        } else {
            result += tmp;
        }
    }

    return result;
}

in_long
in_ddsiSequenceNumberInitFromBuffer(in_ddsiSequenceNumber self,
        in_ddsiDeserializer reader) {
    in_long result, tmp;

    assert(sizeof(*self)==8);
    assert(sizeof(self->high)==4);
    assert(sizeof(self->low)==4);

    result = in_ddsiDeserializerParseLong(reader, &(self->high));
    if (result < 0) {
        result = -1;
    } else {
        tmp = in_ddsiDeserializerParseUlong(reader, &(self->low));
        if (tmp < 0) {
            result = -1;
        } else {
            result += tmp;
        }
    }

    return result;
}

os_boolean in_ddsiSequenceNumberEqual(in_ddsiSequenceNumber self,
        in_ddsiSequenceNumber other) {
    if (self == other)
        return TRUE;
    if (!self)
        return FALSE;
    if (!other)
        return FALSE;

    return (self->high == other->high) && (self->low == other->low);
}

os_boolean
in_ddsiSequenceNumberIsValid(
        in_ddsiSequenceNumber _this)
{
    os_boolean result = OS_FALSE;

    result =
        (_this->high == 0 && _this->low > 0) ||
        (_this->high > 0);

    return result;
}

/**  \brief define an ordering on locators */
c_equality
in_ddsiSequenceNumberCompare(
        in_ddsiSequenceNumber _this,
        in_ddsiSequenceNumber other)
{
    c_equality result = C_NE;

    if (_this->high == other->high) {
        if (_this->low == other->low) {
            result = C_EQ;
        } else  if (_this->low > other->low) {
            result = C_GT;
        } else {
            result = C_LT;
        }
    } else if (_this->high > other->high) {
        result = C_GT;
    } else {
        result = C_LT;
    }

    return result;
}

/* -------------------------------------------------- */
/* ---- in_ddsiSequenceNumberSet -------------------- */
/* -------------------------------------------------- */


static os_boolean
in_ddsiSequenceNumberSetSetNthFlag(
        in_ddsiSequenceNumberSet _this,
        os_uint32 nthBit)
{
    const os_uint32 NBITS_PER_SLOT = 8 * sizeof(os_uint32);
    const os_uint slot = nthBit / NBITS_PER_SLOT; /* elem of [0,7] */
    os_uint32 nthBitInSlot;

    assert(nthBit<256);
    assert(slot < 8);

    /* calculate the bit-position within that slot */
    nthBitInSlot = nthBit - (slot*NBITS_PER_SLOT);

    assert(nthBitInSlot < NBITS_PER_SLOT);

    /* set the flag at specific index */
    _this->bitmap[slot] |= 0xffffffff;/*(1 << nthBitInSlot);*/
    _this->numBits = IN_MAX(_this->numBits, (nthBit+1));
    return OS_TRUE;
}

/* TODO: DBT verify full range */
os_boolean
in_ddsiSequenceNumberSetAdd(
        in_ddsiSequenceNumberSet _this,
        in_ddsiSequenceNumber seqNum)
{
    os_boolean result;
    in_int64 base64;
    in_int64 seqNum64;
    in_int64 deltaN64;
    os_uint32 nthBit;

    base64 = in_ddsiSequenceNumberToInt64(&(_this->bitmapBase));
    seqNum64 = in_ddsiSequenceNumberToInt64(seqNum);
    deltaN64 = seqNum64 - base64;
    /* the new sequence number must not be negative, it
     * must not be less than base and must not exceed the
     * range of 256 bits */
    if (seqNum->high < 0 || deltaN64 < 0 || deltaN64 > 255)
    {
        result = OS_FALSE;
    } else
    {
        nthBit = (os_uint32) (deltaN64 & IN_UINT32_MAX);
        result = in_ddsiSequenceNumberSetSetNthFlag(_this, nthBit);
    }
    return result;
}

/** */
void
in_ddsiSequenceNumberSetDeinit(
        in_ddsiSequenceNumberSet _this)
{
    /* nop */
}

/** */
os_boolean
in_ddsiSequenceNumberSetInit(
        in_ddsiSequenceNumberSet _this,
        in_ddsiSequenceNumber base)
{
    os_boolean result;

    /* A sequence number {0,0} is invalid by definition. But it seems
     * RTI is generating a sequence number {0,0}. So we just check for
     * negative sequence numbers  */
    if (base->high < 0)
    {
        result = OS_FALSE;
    } else
    {
        _this->bitmapBase = *base;
        _this->numBits = 0; /* init with valid value */

        /*bitmap covers 256 bits at max*/
        assert(sizeof(_this->bitmap)*8 == 4*8*8);
        memset(&(_this->bitmap), 0, sizeof(_this->bitmap));

        /* (FR) AFAICS, RTI defines numbits=0 if base==0
        if (base->high != 0 || base->low != 0)
        {
            initialized with valid base, so set lowest bit and define numBits=1

            in_ddsiSequenceNumberSetSetNthFlag(_this, 0);
        }
        */
        result = OS_TRUE;
    }

    return result;
}

/** does not care about trailing padding octets */
os_size_t
in_ddsiSequenceNumberSetSerializedSize(
        in_ddsiSequenceNumberSet _this)
{
    /* Note: this calculation is strongly related to the serialization.
     * Both must be kept in sync with each other. */
    const os_uint32 NBITS_PER_SLOT =
           8*sizeof(os_uint32); /* should be 32 */

    os_size_t result = 0;
    const os_size_t nSlots  =
        /* DDSI-Spec 2.1 declares:
         * there are M=(numBits+31)/32 longs containing the pertinent bits */
        (_this->numBits + (NBITS_PER_SLOT - 1)) / NBITS_PER_SLOT;
        /* RTI workarround: may be 0 in case of numbits==0 and base==0 */
    assert(NBITS_PER_SLOT == 32);
    assert(_this->numBits < 256);

    /* tolerate nSlots==0 */
    assert(nSlots <= 8);

    result = (2*sizeof(os_uint32)) + /* base */
             (1*sizeof(os_uint32)) + /* numBits */
             (nSlots*sizeof(os_uint32)); /* bitlist (seq<ulong,8>) */
    return result;
}

/** */
in_long
in_ddsiSequenceNumberSetSerialize(
        in_ddsiSequenceNumberSet _this,
        in_ddsiSerializer serializer)
{
    const os_uint32 NBITS_PER_SLOT =
         8*sizeof(os_uint32);

    in_long result = -1; /* init with error case -1 */
    in_long total = 0;
    in_long nofOctets;
    os_uint32 index;
    const os_uint32 nSlots =
        /* DDSI-Spec 2.1 declares:
          * there are M=(numBits+31)/32 longs containing the pertinent bits */
         (_this->numBits + (NBITS_PER_SLOT - 1)) / NBITS_PER_SLOT;

    do {
        nofOctets =
            in_ddsiSequenceNumberSerialize(
                    &(_this->bitmapBase),
                    serializer);
        if (nofOctets<0) break;
        total += nofOctets;

        /* serialize the number of bits following */
        nofOctets =
            in_ddsiSerializerAppendUlong(
                    serializer,
                    _this->numBits);
        if (nofOctets<0) break;
        total += nofOctets;

        /* serialize as much slots as required to suffice numbits. If
         * nSlots==0, nothing is serialized. */
        for (index = 0;
             index < 8 && index < nSlots;
             ++index) {
            nofOctets =
                in_ddsiSerializerAppendUlong(
                    serializer,
                    _this->bitmap[index]);
            if (nofOctets<0) break;
            total += nofOctets;
        }

        /* done */

        result = total;
    } while(0);

    return result;
}


static os_boolean
in_ddsiSEquenceNumberSetIsValid(in_ddsiSequenceNumberSet _this)
{
    const os_uint32 NBITS_PER_SLOT = 8*sizeof(os_uint32);
    /* 0<=highestSlotWithBits<8*/
    const os_uint32 highestSlotWithBits = (_this->numBits + (NBITS_PER_SLOT - 1)) / NBITS_PER_SLOT;
    os_boolean result;

    result =
#ifdef IN_WITH_RTI_WORKARROUND
        /* RTI creates numberSeqSet with numBits==0 which is not
         * compliant to DDSI-2.1 spec . In this case the bitmapBase is the
         * only valid sequence number defined by the set. */
        (_this->numBits == 0 &&
         in_ddsiSequenceNumberToInt64(&(_this->bitmapBase)) == 0 &&
         _this->bitmap[0] == 0 &&
         _this->bitmap[1] == 0 &&
         _this->bitmap[2] == 0 &&
         _this->bitmap[3] == 0 &&
         _this->bitmap[4] == 0 &&
         _this->bitmap[5] == 0 &&
         _this->bitmap[6] == 0 &&
         _this->bitmap[7] == 0     ) ||
#endif
        /* this is what the DDSi-2.1 spec defines as valid  */
        (in_ddsiSequenceNumberToInt64(&(_this->bitmapBase)) >= 1 &&
         _this->numBits > 0 &&
         _this->numBits <= 256 &&
         (highestSlotWithBits > 0 && _this->bitmap[highestSlotWithBits-1] > 0)); /* verify the slot has bits defined */
    return result;
}


in_long
in_ddsiSequenceNumberSetInitFromBuffer(in_ddsiSequenceNumberSet _this,
        in_ddsiDeserializer deserializer)
{
    const os_uint32 NBITS_PER_SLOT =
         8*sizeof(os_uint32);

    in_long result = -1; /* init with error case -1 */
    in_long total = 0;
    in_long nofOctets;
    os_uint32 index;
    os_uint32 nSlots = 0; /* depends on numBits to be read from buffer */

    do {
        nofOctets =
             in_ddsiSequenceNumberInitFromBuffer(
                     &(_this->bitmapBase),
                     deserializer);
         if (nofOctets<0) break;
         total += nofOctets;
         /* serialize the number of bits following */
         nofOctets =
             in_ddsiDeserializerParseUlong(
                     deserializer,
                     &(_this->numBits));
         if (nofOctets<0) break;
         total += nofOctets;

         nSlots =
             /* by definition numBits > 0, but tolerate also numBits==0.
              * If numBits==0, do not read any further octets from buffer. */
             (_this->numBits + (NBITS_PER_SLOT - 1)) / NBITS_PER_SLOT;
             /* numBits==0 -> nSlots:=0
              * numBits==1 -> nslots:=1
              * numBits==32 -> nSlots:=1
              * numBits==33 -> nSlots:=2,
              * etc.
              * numBits==256 -> nSlots:=8 */

         assert( /* Slots>=0 && */ nSlots < 8);

         /* verify 256 bits fit into the bitmap */
         assert(sizeof(_this->bitmap)*8 == 4*8*8);
         memset(_this->bitmap, 0, sizeof(_this->bitmap));
         for (index=0; index < 8 && index < nSlots; ++index) {
             nofOctets =
                 in_ddsiDeserializerParseUlong(
                        deserializer,
                        &(_this->bitmap[index]));
                if (nofOctets<0) break;
                total += nofOctets;
         }

         /* verify valid bitmap */
         if (!in_ddsiSEquenceNumberSetIsValid(_this)) break;
         /* done */
         result = total;
    } while(0);

    return result;
}

/* -------------------------------------------------- */
/* ---- in_ddsiFragmentNumber ----------------------- */
/* -------------------------------------------------- */

/*
 OS_CLASS(in_ddsiFragmentNumber);
 OS_STRUCT(in_ddsiFragmentNumber) {
 os_uint32 value;
 };
 */

in_long in_ddsiFragmentNumberSerialize(in_ddsiFragmentNumber self,
        in_ddsiSerializer writer) {
    in_long result;

    result = in_ddsiSerializerAppendUlong(writer, self->value);

    return result;
}

in_long in_ddsiFragmentNumberInitFromBuffer(in_ddsiFragmentNumber self,
        in_ddsiDeserializer reader) {
    in_long result = 0;

    result = in_ddsiDeserializerParseUlong(reader, &(self->value));

    return result;
}

os_boolean in_ddsiFragmentNumberEqual(in_ddsiFragmentNumber self,
        in_ddsiFragmentNumber other) {
    if (self == other)
        return TRUE;
    if (!self)
        return FALSE;
    if (!other)
        return FALSE;

    return self->value == other->value;
}

/* -------------------------------------------------- */
/* ---- in_ddsiEntityName --------------------------- */
/* -------------------------------------------------- */

in_long in_ddsiEntityNameSerialize(in_ddsiEntityName self,
        in_ddsiSerializer writer) {
    assert(self);
    assert(writer);

    return in_ddsiSerializerAppendString(writer, self->value);
}


void
in_ddsiEntityNameInit(
        in_ddsiEntityName self,
        char* name)
{
    self->value = name;
}

in_long in_ddsiEntityNameInitFromBuffer(in_ddsiEntityName self,
        in_ddsiDeserializer reader) {
    os_uint32 strLenOut;
    in_long result;
    assert(self);
    assert(reader);

    /* will reference the string in buffer, does not own the string */
    result = in_ddsiDeserializerReferenceString(reader, &(self->value),
            &strLenOut);

    return result;
}

os_boolean in_ddsiEntityNameEqual(in_ddsiEntityName self,
        in_ddsiEntityName other) {
    os_boolean result;

    if (self == other)
        return TRUE;
    if (!self)
        return FALSE;
    if (!other)
        return FALSE;

    result = (os_strcasecmp(self->value, other->value) == 0);

    return result;
}

/* -------------------------------------------------- */
/* ---- in_ddsiLocator ------------------------------ */
/* -------------------------------------------------- */
void in_ddsiLocatorInitInvalid(in_ddsiLocator self) {
    self->kind = IN_LOCATOR_KIND_INVALID;
    self->port = 0; /* any dummy value would do here */
    in_addressInitAny(&(self->address));
}

void in_ddsiLocatorInit(in_ddsiLocator self, os_int32 kind, os_uint32 port,
        const in_address address) {
    assert(sizeof(kind) == 4);
    assert(sizeof(port) == 4);
    assert(OS_SIZEOF(in_address) == 16);

    self->kind = kind;
    self->port = port;

    if (address) {
        switch (kind) {
        case IN_LOCATOR_KIND_INVALID:
        case IN_LOCATOR_KIND_RESERVED:
            in_addressInitAny(&(self->address));
            break;

        case IN_LOCATOR_KIND_UDPV4:
            assert(in_addressIsIPv4Compatible(address));
        case IN_LOCATOR_KIND_UDPV6:
            in_addressCopy(&(self->address), address);
            break;

        default:
            assert("");
            in_addressInitAny(&(self->address));
        }
    } else {
        in_addressInitAny(&(self->address));
    }
}

/** \brief UDPv4 init function for convenience */
void in_ddsiLocatorInitUDPv4(in_ddsiLocator _this, os_uint32 port,
        const in_addressIPv4 address) {
    OS_STRUCT(in_address) tmp;
    assert(sizeof(port) == 4);
    assert(sizeof(address) == 4);

    in_addressInitIPv4(&tmp, address);

    in_ddsiLocatorInit(_this, IN_LOCATOR_KIND_UDPV4, port, &tmp);
}

/** \brief UDPv6 init function for convenience */
void in_ddsiLocatorInitUDPv6(in_ddsiLocator _this, os_uint32 port,
        const in_addressIPv6 address) {
    OS_STRUCT(in_address) tmp;
    assert(sizeof(port) == 4);
    assert(sizeof(address) == 16);

    in_addressInitIPv6(&tmp, address);

    in_ddsiLocatorInit(_this, IN_LOCATOR_KIND_UDPV6, port, &tmp);
}

void in_ddsiLocatorDeinit(in_ddsiLocator self) {
    /* nop */
}

in_long in_ddsiLocatorSerialize(in_ddsiLocator locator,
        in_ddsiSerializer writer) {
    in_long result, tmp;

    assert(locator);
    assert(writer);
    assert(sizeof(locator->address) == 16);
    assert(locator->kind != IN_LOCATOR_KIND_INVALID);
    assert(locator->kind != IN_LOCATOR_KIND_RESERVED);

    /* dump all 16 octets into the stream */
    result = in_ddsiSerializerAppendLong(writer, locator->kind);
    if (result < 0) {
        result = -1;
    } else {
        tmp = in_ddsiSerializerAppendUlong(writer, locator->port);
        if (tmp < 0) {
            result = -1;
        } else {
            result += tmp;
            tmp = in_ddsiSerializerAppendOctets(writer, in_addressPtr(&(locator->address)), sizeof(locator->address));
            if (tmp < 0) {
                result = -1;
            } else {
                result += tmp;
            }
        }
    }

    return result;
}

in_long
in_ddsiLocatorInitFromBuffer(
	in_ddsiLocator self,
	in_ddsiDeserializer reader)
{
    in_long nofOctets;
    in_long result = 0;

    assert(in_ddsiDeserializerNofUnreadOctets(reader) >= (in_long) sizeof(*self));
    assert(sizeof(in_addressIPv6) == 16);

    /* Note: the first parse operation might perform  up to 3-octet alignment,
     * so that number of parsed octets may range from 24 to 27 en total */
    nofOctets = in_ddsiDeserializerParseLong(reader, &(self->kind));
    if (nofOctets < 0) {
        result = -1;
    } else {
        result = nofOctets;

        /* fast path */
        /* PRE: nofOctets > 0 */

        switch (self->kind) {
        case IN_LOCATOR_KIND_UDPV4:
            /* in case of IPv4 address the first 12 octets are zero
             * and only the remaining 4 octets hold the IPv4 address */
        case IN_LOCATOR_KIND_UDPV6:
            /* parse 4 octets for the port */
            nofOctets = in_ddsiDeserializerParseUlong(reader, &(self->port));
            if (nofOctets < 0) {
                /* parse error */
                result = -1;
            } else {
                /* parse 16 octets */
                in_address address = &(self->address);

                result += nofOctets;

                nofOctets = in_addressInitFromBuffer(address, reader);
                if (nofOctets < 0) {
                    result = -1; /* parse error */
                } else if ((self->kind == IN_LOCATOR_KIND_UDPV4)
                        && !in_addressIsIPv4Compatible(address)) {
                    /* Error: invalid UDPv4 address */
                    result = -1;
                } else if (in_addressIsUnspecified(address)) {
                    result = -1;
                } else {
                    /* succeeded */
                    result += nofOctets;
                }
            }
            /* POST: result >= -1 */
            break;

        default:
            /* unknown locator kind, ignore */
            /* define the object as INVALID and return the number of
             * ocets already parsed. Let the calling operation deal
             * with the case. The calling operation must check the object
             * for kind INVALID */
            in_ddsiLocatorInitInvalid(self);
            /* POST: result == sizeof(os_uint32) */
        }
        /* POST: nofOctets >= 0 */
    }

    return result;
}

void in_ddsiLocatorInitFromSockaddr(in_ddsiLocator self,
        const struct sockaddr *address) {
    struct sockaddr_in *sin = (struct sockaddr_in*) address;
    struct sockaddr_in6 *sin6 = (struct sockaddr_in6*) address;

    assert(address!=NULL);

    switch (sin->sin_family) {
    case AF_INET:
        self->kind = IN_LOCATOR_KIND_UDPV4;
        self->port = ntohs(sin->sin_port);
        /* IPv4 address are represented by 4 octet integer */
        in_addressInitFromInAddr(&(self->address), &(sin->sin_addr));
        break;

    case AF_INET6:
        self->kind = IN_LOCATOR_KIND_UDPV6;
        self->port = ntohs(sin6->sin6_port);
        /* IPv6 address is represented by array of 16 octets */
        in_addressInitFromIn6Addr(&(self->address), &(sin6->sin6_addr));
        break;

    default:
        /* TODO: warning and init as INV_LOCATOR */
        assert(!"not supported");
    }
}

os_boolean in_ddsiLocatorEqual(in_ddsiLocator self, in_ddsiLocator other) {
    os_boolean result = FALSE;

    switch (self->kind) {
    case IN_LOCATOR_KIND_INVALID:
    case IN_LOCATOR_KIND_RESERVED:
        result = (self->kind == other->kind);
        break;

    case IN_LOCATOR_KIND_UDPV4:
    case IN_LOCATOR_KIND_UDPV6:
        result = (self->kind == other->kind) && (self->port == other->port)
                && in_addressEqual(&(self->address), &(other->address));
        break;

    default:
        assert(!"tbd");
    }

    return result;
}

static void locatorToSockaddr(os_int32 kind, /* in_ddsiLocatorKind */
os_uint32 port, const in_address address, /*in_list<in_octet,16>*/
struct sockaddr *dest) {
    struct sockaddr_in *sin;
    struct sockaddr_in6 *sin6;

    assert(dest != NULL);

    switch (kind) {
    case IN_LOCATOR_KIND_INVALID:
    case IN_LOCATOR_KIND_RESERVED:
        assert(!"never reach this");
        memset(dest, 0, sizeof(struct sockaddr));
        break;

    case IN_LOCATOR_KIND_UDPV4:
        sin = (struct sockaddr_in*) dest;
        sin->sin_family = AF_INET;
        sin->sin_port = htons(port);
        /* cast the array to in_addr_t and copy the first 4 octets of the address-array */
        in_addressToInAddr(address, &(sin->sin_addr));
        break;

    case IN_LOCATOR_KIND_UDPV6:
        sin6 = (struct sockaddr_in6*) dest;
        sin6->sin6_family = AF_INET6;
        sin6->sin6_port = htons(port);
        sin6->sin6_flowinfo = 0; /* TODO what is this for? */
        in_addressToIn6Addr(address, &(sin6->sin6_addr));
        sin6->sin6_scope_id = 0; /* TODO handle scopes */

        break;

    default:
        assert(!"unknown locator type");
    }
}

void in_ddsiLocatorToSockaddr(in_ddsiLocator self, struct sockaddr *dest) {
    locatorToSockaddr(self->kind, self->port, &(self->address), dest);
}

void in_ddsiLocatorToSockaddrForAnyAddress(in_ddsiLocator _this,
        struct sockaddr *dest) {
    OS_STRUCT(in_address) unspecifiedAddress = { IN_IPADDRESS_ANY };

    assert(sizeof(unspecifiedAddress) == 16);

    if (in_addressIsIPv4Compatible(&(_this->address))) {
        /* different address has IPv4 format */
        locatorToSockaddr(IN_LOCATOR_KIND_UDPV4, _this->port,
                &unspecifiedAddress, dest);
    } else {
        /* address has IPv6 format */
        locatorToSockaddr(IN_LOCATOR_KIND_UDPV6, _this->port,
                &unspecifiedAddress, dest);
    }
}

os_uint32 in_ddsiLocatorGetPort(in_ddsiLocator self) {
    return self->port;
}

in_address in_ddsiLocatorGetIp(in_ddsiLocator _this) {
    assert(_this);

    return &(_this->address);
}

void in_ddsiLocatorSetPort(in_ddsiLocator self, os_uint32 port) {
    self->port = port;
}

os_int32 in_ddsiLocatorGetKind(in_ddsiLocator self) {
    return self->kind;
}

/**  \brief define an ordering on locators */
static c_equality in_ddsiLocatorCompareUDP(in_ddsiLocator self,
        in_ddsiLocator other) {
    c_equality result = C_NE;

    if (self->kind < other->kind) {
        result = OS_LT;
    } else if (self->kind == other->kind) {
        if (self->port < other->port) {
            result = OS_LT;
        } else if (self->port == other->port) {
            result = in_addressCompare(&(self->address), &(other->address));
        } else {
            result = OS_GT;
        }
    } else {
        result = OS_GT;
    }

    if (self->kind == other->kind && self->port == other->port
    /* in case of IP4 the leading octets must be zero */
    && in_addressEqual(&(self->address), &(other->address))) {
        result = C_EQ;
    }

    return result;
}

c_equality in_ddsiLocatorCompare(in_ddsiLocator self, in_ddsiLocator other) {
    c_equality result = C_NE;

    if (self == other) {
        result = C_EQ;
    } else {
        switch (self->kind) {
        case IN_LOCATOR_KIND_INVALID:
        case IN_LOCATOR_KIND_RESERVED:
            /* Note: comparing objects with this state is just crazy,
             * we should never reach this */
            if (self->kind == other->kind) {
                result = C_EQ;
            }
            break;

        case IN_LOCATOR_KIND_UDPV4:
        case IN_LOCATOR_KIND_UDPV6:
            result = in_ddsiLocatorCompareUDP(self, other);
            break;
        default:
            result = C_NE;
        }
    }

    return result;
}

void in_ddsiLocatorCopy(in_ddsiLocator _this, in_ddsiLocator copyFrom) {
    assert(_this);
    assert(copyFrom);

    /* bitwise copy, make use of builtin-struct copy feature of C */
    _this = copyFrom;
}

/** \brief Verifying locator */
os_boolean in_ddsiLocatorIsValid(in_ddsiLocator _this) {
    const os_uint32 maxPortNr = USHRT_MAX;
    os_boolean result;
    assert(_this);

    switch (_this->kind) {
    case IN_LOCATOR_KIND_INVALID:
    case IN_LOCATOR_KIND_RESERVED:
        result = OS_FALSE;
        break;

    case IN_LOCATOR_KIND_UDPV4:
        result = ((_this->port <= maxPortNr) && (_this->port > 0)
                && in_addressIsIPv4Compatible(&(_this->address))
                && !in_addressIsUnspecified(&(_this->address)));
        break;

    case IN_LOCATOR_KIND_UDPV6:
        result = ((_this->port <= maxPortNr) && (_this->port > 0)
                && !in_addressIsIPv4Compatible(&(_this->address))
                && !in_addressIsUnspecified(&(_this->address)));
        break;
    default:
        result = OS_FALSE;
    }
    return result;
}

/* -------------------------------------------------- */
/* ---- in_ddsiMessageHeader------------------------- */
/* -------------------------------------------------- */

static void in_ddsiMessageHeaderInit(in_ddsiMessageHeader self,
        const in_ddsiProtocolVersion protocolVersion,
        const in_ddsiVendor vendor, const in_ddsiGuidPrefixRef guidPrefix) {
    /* vendor id as declared in DDSI spec. */
    memset(self, 0, sizeof(*self));

    assert(sizeof(in_ddsiGuidPrefix)==12); /* in_octet[12] */
    assert(sizeof(OS_STRUCT(in_ddsiMessageHeader))==20);

    self->protocolId[0] = 'R';
    self->protocolId[1] = 'T';
    self->protocolId[2] = 'P';
    self->protocolId[3] = 'S';
    self->version.major = protocolVersion->major; /* protocol DDSI 2.1  */
    self->version.minor = protocolVersion->minor;
    self->vendor.vendorId[0] = vendor->vendorId[0];
    self->vendor.vendorId[1] = vendor->vendorId[1];
    /* deep copy of type in_octet[12] */
    memcpy(self->guidPrefix, guidPrefix, sizeof(in_ddsiGuidPrefix));
}

static in_long in_ddsiMessageHeaderSerialize(in_ddsiMessageHeader self,
        in_ddsiSerializer serializer) {
    in_long result = 0;
    /* verify no additional alignment necessary */
    assert(in_ddsiSerializerAlign(serializer, IN_DDSI_MESSAGE_ALIGNMENT) == 0);
    assert(in_ddsiSerializerRemainingCapacity(serializer)> (in_long)sizeof(*self));
    assert(sizeof(*self) == 20);

    result = in_ddsiSerializerAppendOctets(serializer, (in_octet*) self,
            (in_long) sizeof(*self));
    if (result < 0) {
        result = -1;
    }

    return result;
}

static void in_ddsiMessageHeaderDeinit(in_ddsiMessageHeader self) {
    /* nop */
}

/** */
in_long in_ddsiMessageHeaderSerializeInstantly(
        const in_ddsiProtocolVersion protocolVersion,
        const in_ddsiVendor vendor, const in_ddsiGuidPrefixRef guidPrefix,
        in_ddsiSerializer serializer) {
    in_long result;
    OS_STRUCT(in_ddsiMessageHeader) header;
    in_ddsiMessageHeaderInit(&header, protocolVersion, vendor, guidPrefix);
    result = in_ddsiMessageHeaderSerialize(&header, serializer);
    in_ddsiMessageHeaderDeinit(&header);
    return result;
}

os_boolean in_ddsiMessageHeaderIsValid(in_ddsiMessageHeader self) {
    in_ddsiGuidPrefix zeroPrefix = IN_GUIDPREFIX_UNKNOWN;

    os_boolean result =
        self->protocolId[0] == 'R' &&
        self->protocolId[1] == 'T' &&
        self->protocolId[2] == 'P' &&
        self->protocolId[3] == 'S' &&
        self->version.major == 2 &&
        self->version.minor == 1 &&
        !in_ddsiGuidPrefixEqual(self->guidPrefix, zeroPrefix);

    return result;
}
in_long in_ddsiMessageHeaderInitFromBuffer(in_ddsiMessageHeader self,
        in_ddsiDeserializer deserializer) {
    in_long result = 0;
    in_long nofOctets = 0;
    /* verify no additional alignment necessary */
    assert(in_ddsiDeserializerAlign(deserializer, IN_DDSI_MESSAGE_ALIGNMENT) == 0);
    assert(sizeof(*self) == 20);

    nofOctets = in_ddsiDeserializerParseOctets(deserializer, (in_octet *) self,
            sizeof(*self));
    if (nofOctets < 0 || !in_ddsiMessageHeaderIsValid(self)) {
        /* parse error or unknown protocol header */
        result = -1;
    } else {
        result = nofOctets;
    }

    return result;
}

/* -------------------------------------------------- */
/* ---- in_ddsiSubmessageHeader ------------------------ */
/* -------------------------------------------------- */

in_long in_ddsiSubmessageHeaderSerializedSize(in_ddsiSerializer cdr) {
    assert(in_ddsiSerializerAlign(cdr, IN_DDSI_SUBMESSAGE_ALIGNMENT) == 0);

    return sizeof(OS_STRUCT(in_ddsiSubmessageHeader));
}

in_long in_ddsiSubmessageHeaderSerializeInstantly(in_ddsiSubmessageKind kind,
        in_ddsiSubmessageFlags flags, os_ushort octetsToNextHeader,
        in_ddsiSerializer cdr) {
    in_long result = 0;
    in_long nofOctets = 0;
    assert (sizeof(os_ushort) == 2);

    /* CHECKME, do we need 4-alignment here? */
    assert(in_ddsiSerializerAlign(cdr, IN_DDSI_SUBMESSAGE_ALIGNMENT) == 0);

    nofOctets = in_ddsiSerializerAppendOctet(cdr, (in_octet) (kind & 0xff));
    if (nofOctets < 0) {
        result = -1;
    } else {
        result += nofOctets;

        nofOctets
                = in_ddsiSerializerAppendOctet(cdr, (in_octet) (flags & 0xff));
        if (nofOctets < 0) {
            result = -1;
        } else {
            result += nofOctets;

            nofOctets = in_ddsiSerializerAppendUshort(cdr,
                    (os_ushort) (octetsToNextHeader & 0xffff));
            if (nofOctets < 0) {
                result = -1;
            } else {
                result += nofOctets;
            }
        }
    }

    return result;
}

in_long in_ddsiSubmessageHeaderInitFromBuffer(in_ddsiSubmessageHeader self,
        in_ddsiDeserializer deserializer) {
    os_boolean bigEndian = TRUE;
    in_long result = 0;
    in_long nOctets = 0;

    nOctets = in_ddsiDeserializerParseOctet(deserializer, &(self->kind));
    if (nOctets < 0) {
        return -1; /* error case */
    } else {
        result += nOctets;

        nOctets = in_ddsiDeserializerParseOctet(deserializer, &(self->flags));
        if (nOctets < 0) {
            result = -1; /* error case */
        } else {
            result += nOctets;
            bigEndian = in_ddsiSubmessageHeaderIsBigEndian(self);

            nOctets = in_ddsiDeserializerParseUshortWithEndianess(deserializer,
                    &(self->octetsToNextHeader), bigEndian);

            if (nOctets < 0) {
                result = -1;
            } else {
                result += nOctets;
            }
        }
    }

    return result;
}

os_boolean in_ddsiSubmessageHeaderIsBigEndian(in_ddsiSubmessageHeader self) {
    /* With RTPS the endianess flag indicates little endianess, so
     * if the endianess flag is set it has the reverse boolean meaning for us*/
    os_boolean result = (in_ddsiSubmessageHeaderHasFlagE(self) ? OS_FALSE
            : OS_TRUE);
    return result;
}

os_boolean in_ddsiSubmessageHeaderHasFlagE(in_ddsiSubmessageHeader _this) {
    os_boolean result = OS_FALSE;
    result = (_this->flags & IN_FLAG_E) ? OS_TRUE : OS_FALSE;
    return result;
}

os_boolean in_ddsiSubmessageHeaderHasFlagI(in_ddsiSubmessageHeader _this) {
    os_boolean result = OS_FALSE;
    switch (_this->kind) {
    case IN_RTPS_DATA:
    case IN_RTPS_DATA_FRAG:
        result = (_this->flags & IN_FLAG_DATA_I) ? OS_TRUE : OS_FALSE;
        break;
    default:
        /* well unexpected type, but we assume same encoding as for DATA */
        result = (_this->flags & IN_FLAG_DATA_I) ? OS_TRUE : OS_FALSE;
    }
    return result;
}

os_boolean in_ddsiSubmessageHeaderHasFlagD(in_ddsiSubmessageHeader _this) {
    os_boolean result;
    switch (_this->kind) {
    case IN_RTPS_DATA:
    case IN_RTPS_DATA_FRAG:
        result = (_this->flags & IN_FLAG_DATA_D) ? OS_TRUE : OS_FALSE;
        break;
    default:
        /* well unexpected type, but we assume same encoding as for DATA */
        result = (_this->flags & IN_FLAG_DATA_D) ? OS_TRUE : OS_FALSE;
    }
    return result;
}

os_boolean in_ddsiSubmessageHeaderHasFlagQ(in_ddsiSubmessageHeader _this) {
    os_boolean result;
    switch (_this->kind) {
    case IN_RTPS_DATA:
        result = (_this->flags & IN_FLAG_DATA_Q) ? OS_TRUE : OS_FALSE;
        break;
    case IN_RTPS_DATA_FRAG:
        result = (_this->flags & IN_FLAG_DATA_FRAG_Q) ? OS_TRUE : OS_FALSE;
        break;
    default:
        /* well unexpected type, but we assume same encoding as for DATA */
        result = (_this->flags & IN_FLAG_DATA_Q) ? OS_TRUE : OS_FALSE;
    }
    return result;
}

os_boolean in_ddsiSubmessageHeaderHasFlagH(in_ddsiSubmessageHeader _this) {
    os_boolean result;
    switch (_this->kind) {
    case IN_RTPS_DATA:
        result = (_this->flags & IN_FLAG_DATA_H) ? OS_TRUE : OS_FALSE;
        break;
    case IN_RTPS_DATA_FRAG:
        result = (_this->flags & IN_FLAG_DATA_FRAG_H) ? OS_TRUE : OS_FALSE;
        break;
    default:
        /* well unexpected type, but we assume same encoding as for DATA */
        result = (_this->flags & IN_FLAG_DATA_H) ? OS_TRUE : OS_FALSE;
    }
    return result;
}

/* -------------------------------------------------- */
/* ---- in_ddsiReliabilityKind ---------------------- */
/* -------------------------------------------------- */

in_long in_ddsiReliabilityKindSerialize(in_ddsiReliabilityKind self,
        in_ddsiSerializer writer) {
    assert(self);

    return in_ddsiSerializerAppendLong(writer, self->value);
}

in_long in_ddsiReliabilityKindInitFromBuffer(in_ddsiReliabilityKind self,
        in_ddsiDeserializer reader) {
    assert(self);

    return in_ddsiDeserializerParseLong(reader, &(self->value));
}

os_boolean in_ddsiReliabilityKindEqual(in_ddsiReliabilityKind self,
        in_ddsiReliabilityKind other) {
    return self->value == other->value;
}

/* -------------------------------------------------- */
/* ---- in_ddsiCount  ------------------------------- */
/* -------------------------------------------------- */

in_long in_ddsiCountSerialize(in_ddsiCount self, in_ddsiSerializer writer) {
    assert(self);

    return in_ddsiSerializerAppendLong(writer, self->value);
}

in_long in_ddsiCountInitFromBuffer(in_ddsiCount self,
        in_ddsiDeserializer reader) {
    assert(self);

    return in_ddsiDeserializerParseLong(reader, &(self->value));
}

os_boolean in_ddsiCountEqual(in_ddsiCount self, in_ddsiCount other) {
    return self->value == other->value;
}

/* -------------------------------------------------- */
/* ---- in_ddsiProtocolVersion ---------------------- */
/* -------------------------------------------------- */

in_long in_ddsiProtocolVersionSerialize(in_ddsiProtocolVersion self,
        in_ddsiSerializer writer) {
    in_long result, tmp;

    assert(self);

    result = in_ddsiSerializerAppendOctet(writer, self->major);
    if (result < 0) {
        result = -1;
    } else {
        tmp = in_ddsiSerializerAppendOctet(writer, self->minor);
        if (tmp < 0) {
            result = -1;
        } else {
            result += tmp;
        }
    }

    return result;
}

in_long in_ddsiProtocolVersionInitFromBuffer(in_ddsiProtocolVersion self,
        in_ddsiDeserializer reader) {
    in_long result, tmp;

    assert(self);

    result = in_ddsiDeserializerParseOctet(reader, &(self->major));
    if (result < 0) {
        result = -1;
    } else {
        tmp = in_ddsiDeserializerParseOctet(reader, &(self->minor));
        if (tmp < 0) {
            result = -1;
        } else {
            result += tmp;
        }
    }
    return result;
}

os_boolean in_ddsiProtocolVersionEqual(in_ddsiProtocolVersion self,
        in_ddsiProtocolVersion other) {
    if (self == other)
        return TRUE;
    if (!self)
        return FALSE;
    if (!other)
        return FALSE;
    return (self->major == other->major) && (self->minor == other->minor);
}

/* -------------------------------------------------- */
/* ---- in_ddsiKeyHashPrefix ------------------------ */
/* -------------------------------------------------- */

in_long in_ddsiKeyHashPrefixSerialize(in_ddsiKeyHashPrefix self,
        in_ddsiSerializer writer) {
    assert(self);
    assert(sizeof(self->value)==12);

    return in_ddsiSerializerAppendOctets(writer, self->value,
            sizeof(self->value));
}

in_long in_ddsiKeyHashPrefixInitFromBuffer(in_ddsiKeyHashPrefix self,
        in_ddsiDeserializer reader) {
    assert(self);
    assert(sizeof(self->value)==12);

    return in_ddsiDeserializerParseOctets(reader, self->value,
            sizeof(self->value));
}

os_boolean in_ddsiKeyHashPrefixEqual(in_ddsiKeyHashPrefix self,
        in_ddsiKeyHashPrefix other) {
    return memcmp(self->value, other->value, sizeof(self->value)) == 0;
}
/* -------------------------------------------------- */
/* ---- in_ddsiKeyHashSuffix ------------------------ */
/* -------------------------------------------------- */

in_long in_ddsiKeyHashSuffixSerialize(in_ddsiKeyHashSuffix self,
        in_ddsiSerializer writer) {
    assert(self);
    assert(sizeof(self->value)==4);

    return in_ddsiSerializerAppendOctets(writer, self->value,
            sizeof(self->value));

}

in_long in_ddsiKeyHashSuffixInitFromBuffer(in_ddsiKeyHashSuffix self,
        in_ddsiDeserializer reader) {
    assert(self);
    assert(sizeof(self->value)==4);

    return in_ddsiDeserializerParseOctets(reader, self->value,
            sizeof(self->value));
}

os_boolean in_ddsiKeyHashSuffixEqual(in_ddsiKeyHashSuffix self,
        in_ddsiKeyHashSuffix other) {
    return memcmp(self->value, other->value, sizeof(self->value)) == 0;
}

/* -------------------------------------------------- */
/* ---- in_ddsiParameterId -------------------------- */
/* -------------------------------------------------- */

in_long in_ddsiParameterIdSerialize(in_ddsiParameterId self,
        in_ddsiSerializer writer) {
    assert(self);
    assert(sizeof(self->value)==2);

    return in_ddsiSerializerAppendUshort(writer, self->value);
}

in_long in_ddsiParameterIdInitFromBuffer(in_ddsiParameterId self,
        in_ddsiDeserializer reader) {
    assert(self);
    assert(sizeof(self->value)==2);

    return in_ddsiDeserializerParseUshort(reader, &(self->value));
}

os_boolean in_ddsiParameterIdEqual(in_ddsiParameterId self,
        in_ddsiParameterId other) {
    return self->value == other->value;
}
/* -------------------------------------------------- */
/* ---- in_ddsiContentFilter* ----------------------- */
/* -------------------------------------------------- */

in_long in_ddsiContentFilterPropertySerialize(
        in_ddsiContentFilterProperty self, in_ddsiSerializer writer) {
    assert(!"not implemented yet");
    return 0;
}

in_long in_ddsiContentFilterPropertyInitFromBuffer(
        in_ddsiContentFilterProperty self, in_ddsiDeserializer reader) {
    assert(!"not implemented yet");
    return 0;
}

in_long in_ddsiContentFilterInfoSerialize(in_ddsiContentFilterInfo self,
        in_ddsiSerializer writer) {
    assert(!"not implemented yet");
    return 0;
}

in_long in_ddsiContentFilterInfoInitFromBuffer(in_ddsiContentFilterInfo self,
        in_ddsiDeserializer reader) {
    assert(!"not implemented yet");
    return 0;
}

/* -------------------------------------------------- */
/* ---- in_ddsiProperty ----------------------------- */
/* -------------------------------------------------- */

in_long in_ddsiPropertySerialize(in_ddsiProperty self, in_ddsiSerializer writer) {
    assert(!"not implemented yet");
    return 0;
}

in_long in_ddsiPropertyInitFromBuffer(in_ddsiProperty self,
        in_ddsiDeserializer reader) {
    assert(!"not implemented yet");
    return 0;
}

os_boolean in_ddsiPropertyEqual(in_ddsiProperty self, in_ddsiProperty other) {
    assert(!"not implemented yet");
    return FALSE;
}

