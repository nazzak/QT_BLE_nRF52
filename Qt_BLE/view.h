#ifndef VIEW_H
#define VIEW_H

#include "defines.h"
#include <QWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include "central.h"

class View : public QWidget
{
    Q_OBJECT

public:
    View(QWidget *parent = 0);
    ~View();

    QVBoxLayout m_vBox;
    QPushButton * m_scan;
    QPushButton * m_reset;
    central * m_control;
public slots:
    void slScanButton();
    void slResetScan();

};

#endif // VIEW_H
