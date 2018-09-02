#include "hbdcmanagerhandler.h"

HBDCManagerHandler::HBDCManagerHandler(ClientWaiter &clientWaiter) : HBDCAppHandler(clientWaiter)
{
}

int HBDCManagerHandler::handleMessage(const QByteArray &msg)
{
    return HBDCAppHandler::handleMessage(msg);
}
