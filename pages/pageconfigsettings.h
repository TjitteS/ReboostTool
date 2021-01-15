#ifndef PAGECONFIGSETTINGS_H
#define PAGECONFIGSETTINGS_H

#include <QWidget>
#include "mpptinterface.h"
#include "configparams.h"

namespace Ui {
class PageconfigSettings;
}

class PageconfigSettings : public QWidget
{
    Q_OBJECT

public:
    explicit PageconfigSettings(QWidget *parent = nullptr);
    ~PageconfigSettings();
    MPPTInterface *mppt() const;
    void SetMPPT(MPPTInterface *mppt);

private:
    Ui::PageconfigSettings *ui;
    MPPTInterface *mMPPT;
};

#endif // PAGECONFIGSETTINGS_H
