#include "smartcardmanage.h"
#include <string>
#include <QDebug>

SmartCardManage::SmartCardManage()
{
    //for SCardListReaders
    hSCardContext = 0;
    ulReadersLength = SCARD_AUTOALLOCATE;
    iReaderCount = 0;

    //for SCardConnect
    for(int i = 0; i < NUMBER_OF_READERS; i ++)
    {
        ProtocolType[i] = SCARD_PROTOCOL_T0 | SCARD_PROTOCOL_T1;
        ActiveProtocol[i] = 0;
        hCardHandle[i] = 0;
        wmemset(szReaders[i], 0, NAME_LENGTH);
    }

    //0:success     other:false
    if(EstablishContext())
    {
        qDebug("EstablishContext false!");
    }else
    {
        qDebug("EstablishContext success!");
    }

    //0:success     other:false
    if(ListReaders())
    {
        qDebug("ListReaders false!");
    }else
    {
        qDebug("ListReaders success!");
    }
}


SmartCardManage::~SmartCardManage()
{
}


LONG SmartCardManage::EstablishContext()
{
    LONG lReturn;
    DWORD dwScope = SCARD_SCOPE_USER;
    LPCVOID pvReserved1 = NULL;
    LPCVOID pvReserved2 = NULL;

    // Establish the context.
    lReturn = SCardEstablishContext(dwScope, pvReserved1, pvReserved2, &hSCardContext);
    if (lReturn != SCARD_S_SUCCESS)
    {
        GetErrorCode(lReturn);
        return FALSE;
    }
    return lReturn;
}


LONG SmartCardManage::ListReaders()
{
    LONG lReturn = -1;
    LPCTSTR pmszGroups = NULL;
    LPTSTR pmszReaders = NULL;

    // List the readers
    lReturn = SCardListReaders(hSCardContext, pmszGroups, (LPTSTR)&pmszReaders, &ulReadersLength);
    if (lReturn != SCARD_S_SUCCESS)
    {
        GetErrorCode(lReturn);
        return FALSE;
    }

    switch( lReturn )
    {
    case SCARD_E_NO_READERS_AVAILABLE:
        qDebug("Reader is not in groups.");
        break;

    case SCARD_S_SUCCESS:
        qDebug("the all reader string length is:%ld", ulReadersLength);

        /*To detect the state of card of all card reader
        //connect the all readers
        unsigned int offset = 0;
        while(ulReadersLength > offset +1)
        {
            wcscpy(szReaders[iReaderCount], (LPCTSTR)pmszReaders + offset);
            qDebug("the reader[%d] is:%ls", iReaderCount, szReaders[iReaderCount]);

            lReturn = SCardConnect(hSCardContext, (LPCTSTR)szReaders[iReaderCount], SCARD_SHARE_SHARED, ProtocolType[iReaderCount], &hCardHandle[iReaderCount], &ActiveProtocol[iReaderCount]);
            if (lReturn != SCARD_S_SUCCESS)
            {
                GetErrorCode(lReturn);
                return FALSE;
            }

            ScardIoRequest[iReaderCount].dwProtocol = ActiveProtocol[iReaderCount];
            ScardIoRequest[iReaderCount].cbPciLength = sizeof(SCARD_IO_REQUEST);

            if (lReturn != SCARD_E_UNKNOWN_READER)
                iReaderCount++;

            if (lReturn == SCARD_S_SUCCESS)
                SCardDisconnect(hCardHandle[iReaderCount], SCARD_UNPOWER_CARD);

            offset += wcslen((wchar_t *) pmszReaders + offset);;
            offset += 1;
        }
        qDebug("Hava %d readers", iReaderCount);
        */
        unsigned int offset = 0;
        while(ulReadersLength > offset +1)
        {
            wcscpy(szReaders[iReaderCount], (LPCTSTR)pmszReaders + offset);
            qDebug("the reader[%d] is:%ls", iReaderCount, szReaders[iReaderCount]);
            iReaderCount++;
            offset += wcslen((wchar_t *) pmszReaders + offset);;
            offset += 1;
        }
        qDebug("Hava %d readers", iReaderCount);
        break;
    }
    return lReturn;
}


VOID SmartCardManage::GetErrorCode(long ret)
{
    switch (ret) {
    case SCARD_E_CANCELLED:
        qDebug("The action was cancelled by an SCardCancel request.");
        break;
    case SCARD_E_CANT_DISPOSE:
        qDebug("The system could not dispose of the media in the requested manner.");
        break;
    case SCARD_E_CARD_UNSUPPORTED:
        qDebug("The smart card does not meet minimal requirements for support.");
        break;
    case SCARD_E_DUPLICATE_READER:
        qDebug("The reader driver didn't produce a unique reader name.");
        break;
    case SCARD_E_INSUFFICIENT_BUFFER:
        qDebug("The data buffer to receive returned data is too small for the returned data.");
        break;
    case SCARD_E_INVALID_ATR:
        qDebug("An ATR obtained from the registry is not a valid ATR string.");
        break;
    case SCARD_E_INVALID_HANDLE:
        qDebug("The supplied handle was invalid.");
        break;
    case SCARD_E_INVALID_PARAMETER:
        qDebug("One or more of the supplied parameters could not be properly interpreted.");
        break;
    case SCARD_E_INVALID_TARGET:
        qDebug("Registry startup information is missing or invalid.");
        break;
    case SCARD_E_INVALID_VALUE:
        qDebug("One or more of the supplied parameters?values could not be properly interpreted.");
        break;
    case SCARD_E_NOT_READY:
        qDebug("The reader or card is not ready to accept commands.");
        break;
    case SCARD_E_NOT_TRANSACTED:
        qDebug("An attempt was made to end a non-existent transaction.");
        break;
    case SCARD_E_NO_MEMORY:
        qDebug("Not enough memory available to complete this command.");
        break;
    case SCARD_E_NO_SERVICE:
        qDebug("The Smart card resource manager is not running.");
        break;
    case SCARD_E_NO_SMARTCARD:
        qDebug("The operation requires a smart card but no smart card is currently in the device.");
        break;
    case SCARD_E_PCI_TOO_SMALL:
        qDebug("The PCI Receive buffer was too small.");
        break;
    case SCARD_E_PROTO_MISMATCH:
        qDebug("The requested protocols are incompatible with the protocol currently in use with the card.");
        break;
    case SCARD_E_READER_UNAVAILABLE:
        qDebug("The specified reader is not currently available for use.");
        break;
    case SCARD_E_READER_UNSUPPORTED:
        qDebug("The reader driver does not meet minimal requirements for support.");
        break;
    case SCARD_E_SERVICE_STOPPED:
        qDebug("The Smart card resource manager has shut down.");
        break;
    case SCARD_E_SHARING_VIOLATION:
        qDebug("The card cannot be accessed because of other connections outstanding.");
        break;
    case SCARD_E_SYSTEM_CANCELLED:
        qDebug("The action was cancelled by the system presumably to log off or shut down.");
        break;
    case SCARD_E_TIMEOUT:
        qDebug("The user-specified timeout value has expired.");
        break;
    case SCARD_E_UNKNOWN_CARD:
        qDebug("The specified card name is not recognized.");
        break;
    case SCARD_E_UNKNOWN_READER:
        qDebug("The specified reader name is not recognized.");
        break;
    case SCARD_F_COMM_ERROR:
        qDebug("An internal communications error has been detected.");
        break;
    case SCARD_F_INTERNAL_ERROR:
        qDebug("An internal consistency check failed.");
        break;
    case SCARD_F_UNKNOWN_ERROR:
        qDebug("An internal error has been detected but the source is unknown.");
        break;
    case SCARD_F_WAITED_TOO_LONG:
        qDebug("An internal consistency timer has expired.");
        break;
    case SCARD_S_SUCCESS:
        qDebug("OK");
        break;
    case SCARD_W_REMOVED_CARD:
        qDebug("The card has been removed so that further communication is not possible.");
        break;
    case SCARD_W_RESET_CARD:
        qDebug("The card has been reset so any shared state information is invalid.");
        break;
    case SCARD_W_UNPOWERED_CARD:
        qDebug("Power has been removed from the card so that further communication is not possible.");
        break;
    case SCARD_W_UNRESPONSIVE_CARD:
        qDebug("The card is not responding to a reset.");
        break;
    case SCARD_W_UNSUPPORTED_CARD:
        qDebug("The reader cannot communicate with the card due to ATR configuration conflicts.");
        break;
    default:
        qDebug("Function returned 0x%X error code.", ret);
        break;
    }
}


BOOL SmartCardManage::CardOpen(int index)
{
    LONG lReturn = -1;

    if(hCardHandle[index] != 0)
    {
        lReturn = SCardDisconnect(hCardHandle[index],SCARD_UNPOWER_CARD);
        if(lReturn != SCARD_S_SUCCESS)
        {
            GetErrorCode(lReturn);
            return lReturn;
        }

        lReturn = SCardConnect(hSCardContext, (LPCTSTR)szReaders[index], SCARD_SHARE_SHARED, ProtocolType[index], &hCardHandle[index], &ActiveProtocol[index]);
        if (lReturn != SCARD_S_SUCCESS)
        {
            GetErrorCode(lReturn);
            return lReturn;
        }

        ScardIoRequest[index].dwProtocol = ActiveProtocol[index];
        ScardIoRequest[index].cbPciLength = sizeof(SCARD_IO_REQUEST);
    }else
    {
        lReturn = SCardConnect(hSCardContext, (LPCTSTR)szReaders[index], SCARD_SHARE_SHARED, ProtocolType[index], &hCardHandle[index], &ActiveProtocol[index]);
        if (lReturn != SCARD_S_SUCCESS)
        {
            GetErrorCode(lReturn);
            return lReturn;
        }

        ScardIoRequest[index].dwProtocol = ActiveProtocol[index];
        ScardIoRequest[index].cbPciLength = sizeof(SCARD_IO_REQUEST);
    }
    return lReturn;
}


BOOL SmartCardManage::CardClose(int index, int type)
{
    LONG lReturn = -1;

    /**************************************************
    Value 										Meaning
    SCARD_LEAVE_CARD               Do not do anything special.
    SCARD_RESET_CARD 			    Reset the card.
    SCARD_UNPOWER_CARD 		Power down the card.
    SCARD_EJECT_CARD 				Eject the card.
    **************************************************/
    switch (type) {
    case SCARD_LEAVE_CARD:
        lReturn = SCardDisconnect(hCardHandle[index], SCARD_LEAVE_CARD);
        break;
    case SCARD_RESET_CARD:
        lReturn = SCardDisconnect(hCardHandle[index], SCARD_RESET_CARD);
        break;
    case SCARD_UNPOWER_CARD:
        lReturn = SCardDisconnect(hCardHandle[index], SCARD_UNPOWER_CARD);
        break;
    case SCARD_EJECT_CARD:
        lReturn = SCardDisconnect(hCardHandle[index], SCARD_EJECT_CARD);
        break;
    default:
        break;
    }

    return lReturn;
}
