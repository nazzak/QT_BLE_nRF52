#include "view.h"

View::View(QWidget *parent)
    : QWidget(parent)
{
    m_central = Q_NULLPTR;

    m_scan = new QPushButton("SCAN", this);
    m_reset = new QPushButton("RESET", this);
    m_textView = new QTextBrowser(this);

    m_vBox.addWidget(m_scan);
    m_vBox.addWidget(m_reset);
    m_vBox.addWidget(m_textView);
    this->setLayout(&m_vBox);


    connect(m_scan, &QPushButton::clicked, this, &View::slScanButton);
    connect(m_reset, &QPushButton::clicked, this, &View::slResetScan);
}

View::~View()
{
    delete m_central;
    delete m_scan;
    delete m_reset;
    // need to delete qtextview

    m_central = Q_NULLPTR;
    m_scan = Q_NULLPTR;
    m_reset = Q_NULLPTR;

    qDebug() << "delete View";
}

void View::slScanButton()
{
    m_scan->setEnabled(false);
    m_central = new central(this);
    connect(m_central, &central::sgToView, this, &View::slToPrint);
    emit m_central->slStartScanning();
}

void View::slResetScan()
{
    qDebug() << "Reset Scanning";
    m_scan->setEnabled(true);
    
    m_textView->clear();

    if(m_central){
        delete m_central;
        m_central = Q_NULLPTR;
    }
}

void View::slToPrint(const QString &_str)
{
    m_textView->append(_str);
}
