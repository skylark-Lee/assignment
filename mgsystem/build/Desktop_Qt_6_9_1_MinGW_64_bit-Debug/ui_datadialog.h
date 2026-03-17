/********************************************************************************
** Form generated from reading UI file 'datadialog.ui'
**
** Created by: Qt User Interface Compiler version 6.9.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DATADIALOG_H
#define UI_DATADIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>

QT_BEGIN_NAMESPACE

class Ui_datadialog
{
public:

    void setupUi(QDialog *datadialog)
    {
        if (datadialog->objectName().isEmpty())
            datadialog->setObjectName("datadialog");
        datadialog->resize(400, 300);

        retranslateUi(datadialog);

        QMetaObject::connectSlotsByName(datadialog);
    } // setupUi

    void retranslateUi(QDialog *datadialog)
    {
        datadialog->setWindowTitle(QCoreApplication::translate("datadialog", "Dialog", nullptr));
    } // retranslateUi

};

namespace Ui {
    class datadialog: public Ui_datadialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DATADIALOG_H
