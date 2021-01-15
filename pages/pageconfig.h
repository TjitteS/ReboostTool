#ifndef PAGECONFIG_H
#define PAGECONFIG_H

#include <QWidget>
#include "mpptinterface.h"

namespace Ui {
class PageConfig;
}

class PageConfig : public QWidget
{
    Q_OBJECT

public:
    explicit PageConfig(QWidget *parent = nullptr);
    ~PageConfig();

    MPPTInterface *mppt() const;
    void setMPPT(MPPTInterface *mppt);

private:
    Ui::PageConfig *ui;
    MPPTInterface *mMPPT;
};

#endif // PAGECONFIG_H
