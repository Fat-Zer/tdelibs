#ifndef KWALLETASYNC_H
#define KWALLETASYNC_H

#include <tqobject.h>

namespace KWallet { class Wallet; }

class WalletReceiver : public QObject
{
	Q_OBJECT
public slots:
	void walletOpened( bool );
};

#endif
