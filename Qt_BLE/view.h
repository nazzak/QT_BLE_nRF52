#ifndef VIEW_H
#define VIEW_H

#include "defines.h"
#include <QWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QTextBrowser>
#include "central.h"

class View : public QWidget
{
    Q_OBJECT

public:
    View(QWidget *parent = 0);
    ~View();

    QVBoxLayout m_vBox;
    QTextBrowser * m_textView;
    QPushButton * m_scan;
    QPushButton * m_reset;
    central * m_central;
public slots:
    void slScanButton();
    void slResetScan();
    void slToPrint(const QString &_str);

};

#endif // VIEW_H
