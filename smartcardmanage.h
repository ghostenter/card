#ifndef SMARTCARDMANAGE_H
#define SMARTCARDMANAGE_H

#define NUMBER_OF_READERS             20
#define NAME_LENGTH                          500

#include <winscard.h>

class SmartCardManage
{
public:
    SmartCardManage();
    ~SmartCardManage();

public:
    VOID GetErrorCode(long ret);
    LONG EstablishContext();
    LONG ListReaders();
    BOOL CardOpen(int index);
    BOOL CardClose(int index, int type);

public:
    //public resources, all the readers use
    SCARDCONTEXT			hSCardContext;
    wchar_t							szReaders[NUMBER_OF_READERS][NAME_LENGTH];
    DWORD							ulReadersLength;
    short								iReaderCount;

private:
    //private resources, each card reader use
    long										ProtocolType[NUMBER_OF_READERS];
    DWORD									ActiveProtocol[NUMBER_OF_READERS];
    SCARDHANDLE					hCardHandle[NUMBER_OF_READERS];
    SCARD_IO_REQUEST			ScardIoRequest[NUMBER_OF_READERS];
};

#endif // SMARTCARDMANAGE_H
