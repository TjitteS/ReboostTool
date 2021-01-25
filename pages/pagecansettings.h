#ifndef PAGECANSETTINGS_H
#define PAGECANSETTINGS_H

#include <QWidget>
#include "mpptinterface.h"


namespace Ui {
class PageCanSettings;
}

class PageCanSettings : public QWidget
{
    Q_OBJECT

public:
    explicit PageCanSettings(QWidget *parent = nullptr);
    ~PageCanSettings();
    MPPTInterface *mppt();
    void SetMPPT(MPPTInterface* mppt);

private:
    Ui::PageCanSettings *ui;
    MPPTInterface *mMPPT;

};

#endif // PAGECANSETTINGS_H
