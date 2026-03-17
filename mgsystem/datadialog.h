#ifndef DATADIALOG_H
#define DATADIALOG_H

#include <QDialog>
#include <QSqlDatabase>
#include <QLineEdit>
#include <QComboBox>
#include <QDateEdit>
#include <QTextEdit>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDoubleValidator>
#include <QIntValidator>
#include <QMessageBox>

// ... 其他代码保持不变 ...
#include <QDialog>
#include <QSqlDatabase>

// namespace Ui {
// class DataDialog;
// }

class DataDialog : public QDialog
{
    Q_OBJECT

public:
    enum DialogType {
        SupplierDialog,
        ProductDialog,
        OrderDialog,
        CustomerDialog,
        EmployeeDialog
    };

    enum Mode { Add, Edit };

    // 构造函数
    explicit DataDialog(QSqlDatabase db, DialogType type, Mode mode, QWidget *parent = nullptr);
    explicit DataDialog(QSqlDatabase db, DialogType type, Mode mode, int keyValue, QWidget *parent = nullptr);
    ~DataDialog();

private slots:
    void onSaveClicked();
    void onCancelClicked();

private:
    void setupUI();
    QWidget* createInputWidget(const QString& fieldName, QWidget* parent);
    void loadData();
    bool validateData();
    bool saveToDatabase();
    bool saveOrderToDatabase();

    //
    bool saveSupplierToDatabase();
    bool saveProductToDatabase();
    bool saveCustomerToDatabase();
    bool saveEmployeeToDatabase();
    bool saveNewOrder();           // 添加新订单
    bool updateExistingOrder();

    // 获取字段名和显示名
    QStringList getFieldNames() const;
    QStringList getFieldDisplayNames() const;
    QString getTableName() const;
    QString getKeyField() const;
    QString getNextSequenceName() const;

    // 特殊字段处理
    QString getStatusDisplayText(const QString& statusCode) const;
    QString getStatusCode(const QString& displayText) const;

private:
    //Ui::DataDialog *ui;
    QSqlDatabase m_db;
    DialogType m_type;
    Mode m_mode;
    int m_keyValue;
    QList<QWidget*> m_inputWidgets;
};

#endif // DATADIALOG_H
