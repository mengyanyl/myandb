#ifndef DMSRECORDID_H_INCLUDED
#define DMSRECORDID_H_INCLUDED

typedef unsigned int PAGEID;
typedef unsigned int SLOTID;

struct dmsRecordID
{
    PAGEID _pageID;
    SLOTID _slotID;
};

#endif // DMSRECORDID_H_INCLUDED
