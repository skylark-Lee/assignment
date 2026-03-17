#include "datadialog.h"
//#include "ui_datadialog.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include <QDebug>
#include <QLineEdit>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QDateEdit>
#include <QComboBox>
#include <QTextEdit>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QPushButton>

DataDialog::DataDialog(QSqlDatabase db, DialogType type, Mode mode, QWidget *parent)
    : QDialog(parent)
    //, ui(new Ui::DataDialog)
    , m_db(db)
    , m_type(type)
    , m_mode(mode)
    , m_keyValue(-1)
{
    setupUI();
}

DataDialog::DataDialog(QSqlDatabase db, DialogType type, Mode mode, int keyValue, QWidget *parent)
    : QDialog(parent)
    //, ui(new Ui::DataDialog)
    , m_db(db)
    , m_type(type)
    , m_mode(mode)
    , m_keyValue(keyValue)
{
    setupUI();
    if (mode == Edit) {
        loadData();
    }
}

DataDialog::~DataDialog()
{
    //delete ui;
}

void DataDialog::setupUI()
{
    // 设置对话框基本属性
    QString title;
    int width = 500;
    int height = 400;

    switch (m_type) {
    case SupplierDialog:
        title = m_mode == Add ? "添加供应商" : "编辑供应商";
        width = 450;
        height = 180;
        break;
    case ProductDialog:
        title = m_mode == Add ? "添加商品" : "编辑商品";
        width = 600;
        height = 550;
        break;
    case OrderDialog:
        title = m_mode == Add ? "添加订单" : "编辑订单";
        width = 600;
        height = 550;
        break;
    case CustomerDialog:
        title = m_mode == Add ? "添加顾客" : "编辑顾客";
        width = 450;
        height = 250;
        break;
    case EmployeeDialog:
        title = m_mode == Add ? "添加员工" : "编辑员工";
        width = 450;
        height = 250;
        break;
    }

    setWindowTitle(title);
    setFixedSize(width, height);

    // 创建主布局
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(15);

    QStringList fieldNames = getFieldNames();
    QStringList displayNames = getFieldDisplayNames();

    // 创建表单区域
    QWidget *formWidget = new QWidget(this);
    QVBoxLayout *formLayout = new QVBoxLayout(formWidget);
    formLayout->setContentsMargins(0, 0, 0, 0);
    formLayout->setSpacing(12);

    // 创建表单
    for (int i = 0; i < fieldNames.size(); i++) {
        QString fieldName = fieldNames[i];
        QString displayName = displayNames[i];

        // 每一行的布局
        QHBoxLayout *rowLayout = new QHBoxLayout();
        rowLayout->setSpacing(10);

        // 左侧：标签
        QLabel *label = new QLabel(displayName + ":", formWidget);
        label->setFixedWidth(100);
        label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        label->setStyleSheet("font-weight: bold; color: #2c3e50;");

        // 右侧：输入控件
        QWidget *inputWidget = createInputWidget(fieldName, formWidget);

        rowLayout->addWidget(label);
        rowLayout->addWidget(inputWidget, 1); // 拉伸因子为1

        formLayout->addLayout(rowLayout);
    }

    // 添加弹性空间
    formLayout->addStretch();

    mainLayout->addWidget(formWidget, 1); // 表单区域占主要空间

    // 按钮区域
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(20);
    buttonLayout->setContentsMargins(0, 10, 0, 0);

    QPushButton *btnSave = new QPushButton("确定", this);
    btnSave->setFixedSize(100, 35);
    btnSave->setStyleSheet(
        "QPushButton {"
        "background-color: #2ecc71;"
        "color: white;"
        "border: none;"
        "border-radius: 5px;"
        "font-size: 14px;"
        "font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "background-color: #27ae60;"
        "}"
        );

    QPushButton *btnCancel = new QPushButton("取消", this);
    btnCancel->setFixedSize(100, 35);
    btnCancel->setStyleSheet(
        "QPushButton {"
        "background-color: #e74c3c;"
        "color: white;"
        "border: none;"
        "border-radius: 5px;"
        "font-size: 14px;"
        "font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "background-color: #c0392b;"
        "}"
        );

    buttonLayout->addStretch();
    buttonLayout->addWidget(btnSave);
    buttonLayout->addWidget(btnCancel);

    mainLayout->addLayout(buttonLayout);

    // 连接信号
    connect(btnSave, &QPushButton::clicked, this, &DataDialog::onSaveClicked);
    connect(btnCancel, &QPushButton::clicked, this, &DataDialog::onCancelClicked);

    setLayout(mainLayout);

    // 如果是编辑模式，加载数据
    if (m_mode == Edit) {
        loadData();
    }
}


void DataDialog::loadData()
{
    if (m_keyValue <= 0) return;

    //qDebug() << "编辑模式加载订单数据，订单号:" << m_keyValue;

    QSqlQuery query(m_db);

    // 加载order表数据
    query.prepare("SELECT ckey, [order status], date, oremarks "
                  "FROM [order] WHERE okey = :okey");
    query.bindValue(":okey", m_keyValue);

    if (!query.exec()) {
        QString error = query.lastError().text();
        //qDebug() << "加载order表数据失败:" << error;
        QMessageBox::warning(this, "加载失败", "加载订单数据失败:\n" + error);
        return;
    }

    int customerId = 0;
    QString status = "P";
    QDate date = QDate::currentDate();
    QString remarks = "";

    if (query.next()) {
        customerId = query.value(0).toInt();
        status = query.value(1).toString();
        date = query.value(2).toDate();
        remarks = query.value(3).toString();
    }

    // 加载odetails表数据
    query.prepare("SELECT pkey, quantity, dprice "
                  "FROM odetails WHERE okey = :okey");
    query.bindValue(":okey", m_keyValue);

    int productId = 0;
    int quantity = 0;
    double unitPrice = 0.0;

    if (query.exec() && query.next()) {
        productId = query.value(0).toInt();
        quantity = query.value(1).toInt();
        unitPrice = query.value(2).toDouble();
    }

    //qDebug() << "加载的数据 - 客户:" << customerId
             //<< "商品:" << productId
             //<< "数量:" << quantity
             //<< "单价:" << unitPrice;

    // 设置控件值
    for (QWidget* widget : m_inputWidgets) {
        QString fieldName = widget->objectName();

        if (fieldName == "ckey") {  // 客户ID
            if (QComboBox* comboBox = qobject_cast<QComboBox*>(widget)) {
                // 在组合框中找到对应的客户
                for (int i = 0; i < comboBox->count(); i++) {
                    if (comboBox->itemData(i).toInt() == customerId) {
                        comboBox->setCurrentIndex(i);
                        break;
                    }
                }
            }
        }
        else if (fieldName == "order status") {  // 订单状态
            if (QComboBox* comboBox = qobject_cast<QComboBox*>(widget)) {
                QString displayText = getStatusDisplayText(status);
                comboBox->setCurrentText(displayText);
            }
        }
        else if (fieldName == "date") {  // 订单日期
            if (QDateEdit* dateEdit = qobject_cast<QDateEdit*>(widget)) {
                dateEdit->setDate(date);
            }
        }
        else if (fieldName == "pkey") {  // 商品ID
            if (QComboBox* comboBox = qobject_cast<QComboBox*>(widget)) {
                for (int i = 0; i < comboBox->count(); i++) {
                    if (comboBox->itemData(i).toInt() == productId) {
                        comboBox->setCurrentIndex(i);
                        break;
                    }
                }
            }
        }
        else if (fieldName == "quantity") {  // 数量
            if (QLineEdit* lineEdit = qobject_cast<QLineEdit*>(widget)) {
                lineEdit->setText(QString::number(quantity));
            }
        }
        else if (fieldName == "dprice") {  // 单价
            if (QLineEdit* lineEdit = qobject_cast<QLineEdit*>(widget)) {
                lineEdit->setText(QString::number(unitPrice, 'f', 2));
            }
        }
        else if (fieldName == "oremarks") {  // 备注
            if (QTextEdit* textEdit = qobject_cast<QTextEdit*>(widget)) {
                textEdit->setText(remarks);
            }
        }
    }
}
bool DataDialog::validateData()
{
    for (QWidget *widget : m_inputWidgets) {
        QString fieldName = widget->objectName();

        if (fieldName == "sname" || fieldName == "pname" || fieldName == "cname" || fieldName == "ename") {
            QLineEdit *lineEdit = qobject_cast<QLineEdit*>(widget);
            if (lineEdit && lineEdit->text().trimmed().isEmpty()) {
                QString fieldType = fieldName.left(1).toUpper() + fieldName.mid(1).replace("name", "名称");
                QMessageBox::warning(this, "输入错误", QString("%1不能为空").arg(fieldType));
                lineEdit->setFocus();
                return false;
            }
        }
        else if (fieldName == "type" || fieldName == "position") {
            QComboBox *comboBox = qobject_cast<QComboBox*>(widget);
            if (comboBox && comboBox->currentText().trimmed().isEmpty()) {
                QString fieldType = fieldName == "type" ? "类型" : "职位";
                QMessageBox::warning(this, "输入错误", QString("%1不能为空").arg(fieldType));
                comboBox->setFocus();
                return false;
            }
        }
        else if (fieldName == "password") {
            QLineEdit *lineEdit = qobject_cast<QLineEdit*>(widget);
            if (lineEdit && lineEdit->text().trimmed().isEmpty()) {
                QMessageBox::warning(this, "输入错误", "密码不能为空");
                lineEdit->setFocus();
                return false;
            }
        }
    }
    return true;
}
bool DataDialog::saveToDatabase()
{
    qDebug() << "DataDialog::saveToDatabase() 被调用，类型:" << m_type;

    switch (m_type) {
    case SupplierDialog:
        return saveSupplierToDatabase();

    case ProductDialog:
        return saveProductToDatabase();

    case OrderDialog:
        return saveOrderToDatabase();

    case CustomerDialog:
        return saveCustomerToDatabase();

    case EmployeeDialog:
        return saveEmployeeToDatabase();

    default:
        qDebug() << "未处理的对话框类型:" << m_type;
        return false;
    }
}
// 保存供应商
bool DataDialog::saveSupplierToDatabase()
{
    qDebug() << "保存供应商数据...";

    QString supplierName = "";

    // 获取输入的值
    for (QWidget* widget : m_inputWidgets) {
        if (widget->objectName() == "sname") {
            if (QLineEdit* lineEdit = qobject_cast<QLineEdit*>(widget)) {
                supplierName = lineEdit->text().trimmed();
            }
        }
    }

    if (supplierName.isEmpty()) {
        QMessageBox::warning(this, "错误", "供应商名称不能为空");
        return false;
    }

    QSqlQuery query(m_db);

    if (m_mode == Add) {
        // 获取下一个供应商ID
        query.exec("SELECT NEXT VALUE FOR seq_supplier_key");
        int supplierId = 1000;
        if (query.next()) {
            supplierId = query.value(0).toInt();
        }

        query.prepare("INSERT INTO supplier (skey, sname) VALUES (:skey, :sname)");
        query.bindValue(":skey", supplierId);
        query.bindValue(":sname", supplierName);
    } else {
        query.prepare("UPDATE supplier SET sname = :sname WHERE skey = :skey");
        query.bindValue(":sname", supplierName);
        query.bindValue(":skey", m_keyValue);
    }

    if (!query.exec()) {
        QMessageBox::warning(this, "保存失败", "保存供应商失败: " + query.lastError().text());
        return false;
    }

    return true;
}

// 保存商品
bool DataDialog::saveProductToDatabase()
{
    qDebug() << "保存商品数据...";

    QString productName = "";
    QString type = "";
    double price = 0.0;
    int stock = 0;
    int sold = 0;
    QString remarks = "";

    // 获取输入的值
    for (QWidget* widget : m_inputWidgets) {
        QString fieldName = widget->objectName();

        if (fieldName == "pname") {  // 商品名称
            if (QLineEdit* lineEdit = qobject_cast<QLineEdit*>(widget)) {
                productName = lineEdit->text().trimmed();
            }
        }
        else if (fieldName == "type") {  // 商品类型
            if (QComboBox* comboBox = qobject_cast<QComboBox*>(widget)) {
                type = comboBox->currentText().trimmed();
            }
        }
        else if (fieldName == "sprice") {  // 售价
            if (QLineEdit* lineEdit = qobject_cast<QLineEdit*>(widget)) {
                price = lineEdit->text().toDouble();
            }
        }
        else if (fieldName == "stock") {  // 库存
            if (QLineEdit* lineEdit = qobject_cast<QLineEdit*>(widget)) {
                stock = lineEdit->text().toInt();
            }
        }
        else if (fieldName == "sold") {  // 已售数量
            if (QLineEdit* lineEdit = qobject_cast<QLineEdit*>(widget)) {
                sold = lineEdit->text().toInt();
            }
        }
        else if (fieldName == "premarks") {  // 备注
            if (QTextEdit* textEdit = qobject_cast<QTextEdit*>(widget)) {
                remarks = textEdit->toPlainText().trimmed();
            }
        }
    }

    // 验证必填字段
    if (productName.isEmpty()) {
        QMessageBox::warning(this, "错误", "商品名称不能为空");
        return false;
    }

    if (type.isEmpty()) {
        QMessageBox::warning(this, "错误", "商品类型不能为空");
        return false;
    }

    if (price <= 0) {
        QMessageBox::warning(this, "错误", "售价必须大于0");
        return false;
    }

    if (stock < 0) {
        QMessageBox::warning(this, "错误", "库存不能为负数");
        return false;
    }

    if (sold < 0) {
        QMessageBox::warning(this, "错误", "已售数量不能为负数");
        return false;
    }

    QSqlQuery query(m_db);

    if (m_mode == Add) {
        // 获取下一个商品ID
        query.exec("SELECT NEXT VALUE FOR seq_product_key");
        int productId = 1000;
        if (query.next()) {
            productId = query.value(0).toInt();
        }

        query.prepare("INSERT INTO product (pkey, pname, type, sprice, stock, sold, premarks) "
                      "VALUES (:pkey, :pname, :type, :sprice, :stock, :sold, :premarks)");
        query.bindValue(":pkey", productId);
    } else {
        query.prepare("UPDATE product SET pname = :pname, type = :type, sprice = :sprice, "
                      "stock = :stock, sold = :sold, premarks = :premarks "
                      "WHERE pkey = :pkey");
        query.bindValue(":pkey", m_keyValue);
    }

    query.bindValue(":pname", productName);
    query.bindValue(":type", type);
    query.bindValue(":sprice", price);
    query.bindValue(":stock", stock);
    query.bindValue(":sold", sold);
    query.bindValue(":premarks", remarks);

    if (!query.exec()) {
        QMessageBox::warning(this, "保存失败", "保存商品失败: " + query.lastError().text());
        return false;
    }

    return true;
}

// 保存顾客
bool DataDialog::saveCustomerToDatabase()
{
    qDebug() << "保存顾客数据...";

    QString customerName = "";
    QString remarks = "";

    // 获取输入的值
    for (QWidget* widget : m_inputWidgets) {
        QString fieldName = widget->objectName();

        if (fieldName == "cname") {  // 顾客姓名
            if (QLineEdit* lineEdit = qobject_cast<QLineEdit*>(widget)) {
                customerName = lineEdit->text().trimmed();
            }
        }
        else if (fieldName == "cremarks") {  // 备注
            if (QTextEdit* textEdit = qobject_cast<QTextEdit*>(widget)) {
                remarks = textEdit->toPlainText().trimmed();
            }
        }
    }

    if (customerName.isEmpty()) {
        QMessageBox::warning(this, "错误", "顾客姓名不能为空");
        return false;
    }

    QSqlQuery query(m_db);

    if (m_mode == Add) {
        // 获取下一个顾客ID
        query.exec("SELECT NEXT VALUE FOR seq_customer_key");
        int customerId = 1000;
        if (query.next()) {
            customerId = query.value(0).toInt();
        }

        query.prepare("INSERT INTO customer (ckey, cname, cremarks) "
                      "VALUES (:ckey, :cname, :cremarks)");
        query.bindValue(":ckey", customerId);
    } else {
        query.prepare("UPDATE customer SET cname = :cname, cremarks = :cremarks "
                      "WHERE ckey = :ckey");
        query.bindValue(":ckey", m_keyValue);
    }

    query.bindValue(":cname", customerName);
    query.bindValue(":cremarks", remarks);

    if (!query.exec()) {
        QMessageBox::warning(this, "保存失败", "保存顾客失败: " + query.lastError().text());
        return false;
    }

    return true;
}

// 保存员工
bool DataDialog::saveEmployeeToDatabase()
{
    qDebug() << "保存员工数据...";

    QString employeeName = "";
    QString position = "";
    QString password = "";

    // 获取输入的值
    for (QWidget* widget : m_inputWidgets) {
        QString fieldName = widget->objectName();

        if (fieldName == "ename") {  // 员工姓名
            if (QLineEdit* lineEdit = qobject_cast<QLineEdit*>(widget)) {
                employeeName = lineEdit->text().trimmed();
            }
        }
        else if (fieldName == "position") {  // 职位
            if (QComboBox* comboBox = qobject_cast<QComboBox*>(widget)) {
                position = comboBox->currentText().trimmed();
            }
        }
        else if (fieldName == "password") {  // 密码
            if (QLineEdit* lineEdit = qobject_cast<QLineEdit*>(widget)) {
                password = lineEdit->text().trimmed();
            }
        }
    }

    if (employeeName.isEmpty()) {
        QMessageBox::warning(this, "错误", "员工姓名不能为空");
        return false;
    }

    if (position.isEmpty()) {
        QMessageBox::warning(this, "错误", "职位不能为空");
        return false;
    }

    if (password.isEmpty()) {
        QMessageBox::warning(this, "错误", "密码不能为空");
        return false;
    }

    QSqlQuery query(m_db);

    if (m_mode == Add) {
        // 获取下一个员工ID
        query.exec("SELECT NEXT VALUE FOR seq_employee_key");
        int employeeId = 1000;
        if (query.next()) {
            employeeId = query.value(0).toInt();
        }

        query.prepare("INSERT INTO employee (ekey, ename, position, password) "
                      "VALUES (:ekey, :ename, :position, :password)");
        query.bindValue(":ekey", employeeId);
    } else {
        query.prepare("UPDATE employee SET ename = :ename, position = :position, "
                      "password = :password WHERE ekey = :ekey");
        query.bindValue(":ekey", m_keyValue);
    }

    query.bindValue(":ename", employeeName);
    query.bindValue(":position", position);
    query.bindValue(":password", password);

    if (!query.exec()) {
        QMessageBox::warning(this, "保存失败", "保存员工失败: " + query.lastError().text());
        return false;
    }

    return true;
}


void DataDialog::onSaveClicked()
{
    if (!validateData()) {
        return;
    }

    if (saveToDatabase()) {
        QMessageBox::information(this, "成功",
                                 m_mode == Add ? "数据添加成功！" : "数据更新成功！");
        accept();
    }
}

void DataDialog::onCancelClicked()
{
    reject();
}

QStringList DataDialog::getFieldNames() const
{
    switch (m_type) {
    case SupplierDialog:
        return {"sname"};  // 供应商只需要名称

    case ProductDialog:
        return {"pname", "type", "sprice", "stock", "sold", "premarks"};
        // 1.商品名称 2.类型 3.售价 4.库存 5.已售 6.备注

    case OrderDialog:
        return {"okey", "ckey", "order status", "date",
                "pkey", "quantity", "dprice", "oremarks"};

    case CustomerDialog:
        return {"cname", "cremarks"};
        // 1.顾客姓名 2.备注

    case EmployeeDialog:
        return {"ename", "position", "password"};
        // 1.员工姓名 2.职位 3.密码
    }
    return {};
}

QStringList DataDialog::getFieldDisplayNames() const
{
    switch (m_type) {
    case SupplierDialog:
        return {"供应商名称"};

    case ProductDialog:
        return {"商品名称", "商品类型", "售价(¥)", "库存", "已售数量", "备注"};

    case OrderDialog:
        return {"订单号", "客户", "订单状态", "订单日期",
                "商品", "数量", "单价(¥)", "备注"};

    case CustomerDialog:
        return {"顾客姓名", "备注"};

    case EmployeeDialog:
        return {"员工姓名", "职位", "密码"};
    }
    return {};
}

QString DataDialog::getTableName() const
{
    switch (m_type) {
    case SupplierDialog: return "supplier";
    case ProductDialog: return "product";
    case OrderDialog: return "[order]";
    case CustomerDialog: return "customer";
    case EmployeeDialog: return "employee";
    }
    return "";
}

QString DataDialog::getKeyField() const
{
    switch (m_type) {
    case SupplierDialog: return "skey";
    case ProductDialog: return "pkey";
    case OrderDialog: return "okey";
    case CustomerDialog: return "ckey";
    case EmployeeDialog: return "ekey";
    }
    return "";
}

QString DataDialog::getNextSequenceName() const
{
    switch (m_type) {
    case SupplierDialog: return "seq_supplier_key";
    case ProductDialog: return "seq_product_key";
    case OrderDialog: return "seq_order_key";
    case CustomerDialog: return "seq_customer_key";
    case EmployeeDialog: return "seq_employee_key";
    }
    return "";
}

QString DataDialog::getStatusDisplayText(const QString& statusCode) const
{
    if (statusCode == "F") return "已完成";
    else if (statusCode == "P") return "进行中";
    else if (statusCode == "E") return "已取消";
    return statusCode;
}

QString DataDialog::getStatusCode(const QString& displayText) const
{
    if (displayText == "已完成") return "F";
    else if (displayText == "进行中") return "P";
    else if (displayText == "已取消") return "E";
    return displayText;
}
//
// QString DataDialog::getOdetailsSequenceName() const
// {
//     return "seq_order_detail_key";  // ✅ 和您的SQL一致
// }
QWidget* DataDialog::createInputWidget(const QString& fieldName, QWidget* parent)
{
    QWidget *inputWidget = nullptr;

    // 1. 主键字段处理
    if (fieldName == "okey" ||fieldName == "ekey" || fieldName == "skey") {
        QLineEdit *lineEdit = new QLineEdit(parent);
        lineEdit->setObjectName(fieldName);
        lineEdit->setReadOnly(true);
        if (m_mode == Add) {
            lineEdit->setText("自动生成");
        } else {
            lineEdit->setText(QString::number(m_keyValue));
        }
        lineEdit->setStyleSheet("QLineEdit { background-color: #f5f5f5; }");
        inputWidget = lineEdit;
    }
    // 2. 订单相关字段
    else if (fieldName == "pkey") {  // 商品ID
        //qDebug() << "创建商品选择下拉框";

        QComboBox *comboBox = new QComboBox(parent);
        comboBox->setObjectName("pkey");
        QSqlQuery query(m_db);
        if (query.exec("SELECT pkey, pname, sprice FROM product ORDER BY pname")) {
            comboBox->addItem("请选择商品...", 0);

            while (query.next()) {
                int productId = query.value(0).toInt();
                QString productName = query.value(1).toString();
                double price = query.value(2).toDouble();
                QString displayText = QString("%1 - %2 (¥%3)")
                                          .arg(productId)
                                          .arg(productName)
                                          .arg(price, 0, 'f', 2);
                comboBox->addItem(displayText, productId);
            }
        } else {
            //qDebug() << "加载商品失败:" << query.lastError().text();
            comboBox->addItem("加载商品失败", 0);
        }

        comboBox->setCurrentIndex(0);
        inputWidget = comboBox;
    }
    else if (fieldName == "quantity") {  // 数量
        QLineEdit *lineEdit = new QLineEdit(parent);
        lineEdit->setObjectName("quantity");
        lineEdit->setPlaceholderText("请输入数量");
        QIntValidator *validator = new QIntValidator(1, 9999, lineEdit);
        lineEdit->setValidator(validator);
        lineEdit->setText("1");  // 默认数量为1
        inputWidget = lineEdit;
    }
    else if (fieldName == "dprice") {  // 单价
        QLineEdit *lineEdit = new QLineEdit(parent);
        lineEdit->setObjectName("dprice");
        lineEdit->setPlaceholderText("例如: 19.99");
        QDoubleValidator *validator = new QDoubleValidator(0, 999999, 2, lineEdit);
        lineEdit->setValidator(validator);
        inputWidget = lineEdit;
    }
    else if (fieldName == "ckey") {  // ✅ 客户ID
        //qDebug() << "创建客户ID字段，创建下拉框";

        QComboBox *comboBox = new QComboBox(parent);
        comboBox->setObjectName("ckey");

        // 加载已有客户
        QSqlQuery query(m_db);
        if (query.exec("SELECT ckey, cname FROM customer ORDER BY ckey")) {
            comboBox->addItem("请选择客户...", 0);

            while (query.next()) {
                int customerId = query.value(0).toInt();
                QString customerName = query.value(1).toString();
                QString displayText = QString("%1 - %2").arg(customerId).arg(customerName);
                comboBox->addItem(displayText, customerId);
            }

            //qDebug() << "加载了" << comboBox->count() - 1 << "个客户";
        } else {
            //qDebug() << "加载客户失败:" << query.lastError().text();
            comboBox->addItem("加载客户失败", 0);
        }

        comboBox->setCurrentIndex(0);
        inputWidget = comboBox;

        //qDebug() << "客户ID下拉框创建完成";
    }
    else if (fieldName == "order status") {  // 订单状态
        QComboBox *comboBox = new QComboBox(parent);
        comboBox->setObjectName("order status");
        comboBox->addItems({"进行中", "已完成", "已取消"});
        comboBox->setCurrentText("进行中");
        inputWidget = comboBox;
    }
    else if (fieldName == "date") {  // 订单日期
        QDateEdit *dateEdit = new QDateEdit(parent);
        dateEdit->setObjectName("date");
        dateEdit->setDate(QDate::currentDate());
        dateEdit->setCalendarPopup(true);
        dateEdit->setDisplayFormat("yyyy-MM-dd");
        inputWidget = dateEdit;
    }
    else if (fieldName == "oremarks") {  // 订单备注
        QTextEdit *textEdit = new QTextEdit(parent);
        textEdit->setObjectName("oremarks");
        textEdit->setMaximumHeight(60);
        textEdit->setPlaceholderText("请输入订单备注...");
        inputWidget = textEdit;
    }
    // 3. 商品相关字段
    else if (fieldName == "pname") {  // 商品名称
        QLineEdit *lineEdit = new QLineEdit(parent);
        lineEdit->setObjectName(fieldName);
        lineEdit->setPlaceholderText("请输入商品名称");
        inputWidget = lineEdit;
    }
    else if (fieldName == "type") {  // 商品类型
        QComboBox *comboBox = new QComboBox(parent);
        comboBox->setObjectName(fieldName);
        comboBox->setEditable(true);
        comboBox->addItems({"电子产品", "食品", "服装", "日用品", "办公用品"});
        inputWidget = comboBox;
    }
    else if (fieldName == "sprice") {  // 售价
        QLineEdit *lineEdit = new QLineEdit(parent);
        lineEdit->setObjectName(fieldName);
        lineEdit->setPlaceholderText("例如: 19.99");
        QDoubleValidator *validator = new QDoubleValidator(0, 999999, 2, lineEdit);
        lineEdit->setValidator(validator);
        inputWidget = lineEdit;
    }
    else if (fieldName == "stock") {  // 库存
        QLineEdit *lineEdit = new QLineEdit(parent);
        lineEdit->setObjectName(fieldName);
        lineEdit->setPlaceholderText("例如: 100");
        QIntValidator *validator = new QIntValidator(0, 999999, lineEdit);
        lineEdit->setValidator(validator);
        inputWidget = lineEdit;
    }
    else if (fieldName == "sold") {  // 已售数量
        QLineEdit *lineEdit = new QLineEdit(parent);
        lineEdit->setObjectName(fieldName);
        lineEdit->setPlaceholderText("例如: 50");
        QIntValidator *validator = new QIntValidator(0, 999999, lineEdit);
        lineEdit->setValidator(validator);
        inputWidget = lineEdit;
    }
    else if (fieldName == "premarks") {  // 商品备注
        QTextEdit *textEdit = new QTextEdit(parent);
        textEdit->setObjectName(fieldName);
        textEdit->setMaximumHeight(60);
        textEdit->setPlaceholderText("请输入商品备注...");
        inputWidget = textEdit;
    }
    // 4. 顾客相关字段
    else if (fieldName == "cname") {  // 顾客姓名
        QLineEdit *lineEdit = new QLineEdit(parent);
        lineEdit->setObjectName(fieldName);
        lineEdit->setPlaceholderText("请输入顾客姓名");
        inputWidget = lineEdit;
    }
    else if (fieldName == "cremarks") {  // 顾客备注
        QTextEdit *textEdit = new QTextEdit(parent);
        textEdit->setObjectName(fieldName);
        textEdit->setMaximumHeight(60);
        textEdit->setPlaceholderText("请输入顾客备注...");
        inputWidget = textEdit;
    }
    // 5. 员工相关字段
    else if (fieldName == "ename") {  // 员工姓名
        QLineEdit *lineEdit = new QLineEdit(parent);
        lineEdit->setObjectName(fieldName);
        lineEdit->setPlaceholderText("请输入员工姓名");
        inputWidget = lineEdit;
    }
    else if (fieldName == "position") {  // 职位
        QComboBox *comboBox = new QComboBox(parent);
        comboBox->setObjectName(fieldName);
        comboBox->setEditable(true);
        comboBox->addItems({"管理员", "职员"});
        inputWidget = comboBox;
    }
    else if (fieldName == "password") {  // 密码
        QLineEdit *lineEdit = new QLineEdit(parent);
        lineEdit->setObjectName(fieldName);
        lineEdit->setEchoMode(QLineEdit::Password);
        lineEdit->setPlaceholderText("请输入密码");
        inputWidget = lineEdit;
    }
    // 6. 供应商相关字段
    else if (fieldName == "sname") {  // 供应商名称
        QLineEdit *lineEdit = new QLineEdit(parent);
        lineEdit->setObjectName(fieldName);
        lineEdit->setPlaceholderText("请输入供应商名称");
        inputWidget = lineEdit;
    }
    // 7. 默认情况
    else {
        QLineEdit *lineEdit = new QLineEdit(parent);
        lineEdit->setObjectName(fieldName);
        lineEdit->setPlaceholderText("请输入" + fieldName);
        inputWidget = lineEdit;
    }

    // 设置统一的样式
    if (QLineEdit *lineEdit = qobject_cast<QLineEdit*>(inputWidget)) {
        lineEdit->setStyleSheet(
            "QLineEdit {"
            "padding: 8px;"
            "border: 1px solid #bdc3c7;"
            "border-radius: 4px;"
            "font-size: 13px;"
            "}"
            "QLineEdit:focus {"
            "border: 1px solid #3498db;"
            "}"
            );
    }
    else if (QComboBox *comboBox = qobject_cast<QComboBox*>(inputWidget)) {
        comboBox->setStyleSheet(
            "QComboBox {"
            "padding: 6px;"
            "border: 1px solid #bdc3c7;"
            "border-radius: 4px;"
            "background-color: white;"
            "}"
            "QComboBox:focus {"
            "border: 1px solid #3498db;"
            "}"
            );
    }
    else if (QDateEdit *dateEdit = qobject_cast<QDateEdit*>(inputWidget)) {
        dateEdit->setStyleSheet(
            "QDateEdit {"
            "padding: 6px;"
            "border: 1px solid #bdc3c7;"
            "border-radius: 4px;"
            "}"
            "QDateEdit:focus {"
            "border: 1px solid #3498db;"
            "}"
            );
    }
    else if (QTextEdit *textEdit = qobject_cast<QTextEdit*>(inputWidget)) {
        textEdit->setStyleSheet(
            "QTextEdit {"
            "padding: 8px;"
            "border: 1px solid #bdc3c7;"
            "border-radius: 4px;"
            "font-size: 13px;"
            "}"
            "QTextEdit:focus {"
            "border: 1px solid #3498db;"
            "}"
            );
    }

    m_inputWidgets.append(inputWidget);
    return inputWidget;
}
bool DataDialog::saveOrderToDatabase()
{
    qDebug() << "保存订单数据，模式:" << (m_mode == Add ? "添加" : "编辑");

    if (m_mode == Add) {
        return saveNewOrder();  // 添加模式
    } else {
        return updateExistingOrder();  // 编辑模式
    }
}
bool DataDialog::saveNewOrder()
{
    qDebug() << "========== saveOrderToDatabase 开始 ==========";

    if (m_mode == Add) {
        // 1. 获取下一个订单号
        QSqlQuery seqQuery(m_db);
        if (!seqQuery.exec("SELECT NEXT VALUE FOR seq_order_key")) {
            qDebug() << "获取订单序列失败:" << seqQuery.lastError().text();
            return false;
        }

        int orderId = 1000;
        if (seqQuery.next()) {
            orderId = seqQuery.value(0).toInt();
        }
        m_keyValue = orderId;
        qDebug() << "生成的订单号:" << orderId;

        // 2. 收集字段值
        int customerId = 0;
        QString status = "P";
        QDate date = QDate::currentDate();
        int productId = 0;
        int quantity = 0;
        double unitPrice = 0.0;
        QString remarks = "";

        qDebug() << "开始收集字段值，控件数量:" << m_inputWidgets.size();

        for (QWidget* widget : m_inputWidgets) {
            QString fieldName = widget->objectName();

            if (fieldName == "ckey") {  // 客户ID
                if (QComboBox* comboBox = qobject_cast<QComboBox*>(widget)) {
                    customerId = comboBox->currentData().toInt();
                    qDebug() << "客户ID:" << customerId;
                }
            }
            else if (fieldName == "order status") {  // 订单状态
                if (QComboBox* comboBox = qobject_cast<QComboBox*>(widget)) {
                    status = getStatusCode(comboBox->currentText());
                    qDebug() << "订单状态:" << status;
                }
            }
            else if (fieldName == "date") {  // 订单日期
                if (QDateEdit* dateEdit = qobject_cast<QDateEdit*>(widget)) {
                    date = dateEdit->date();
                    qDebug() << "订单日期:" << date.toString("yyyy-MM-dd");
                }
            }
            else if (fieldName == "pkey") {  // 商品ID
                if (QComboBox* comboBox = qobject_cast<QComboBox*>(widget)) {
                    productId = comboBox->currentData().toInt();
                    qDebug() << "商品ID:" << productId;
                }
            }
            else if (fieldName == "quantity") {  // 数量
                if (QLineEdit* lineEdit = qobject_cast<QLineEdit*>(widget)) {
                    quantity = lineEdit->text().toInt();
                    qDebug() << "数量:" << quantity;
                }
            }
            else if (fieldName == "dprice") {  // 单价
                if (QLineEdit* lineEdit = qobject_cast<QLineEdit*>(widget)) {
                    unitPrice = lineEdit->text().toDouble();
                    qDebug() << "单价:" << unitPrice;
                }
            }
            else if (fieldName == "oremarks") {  // 备注
                if (QTextEdit* textEdit = qobject_cast<QTextEdit*>(widget)) {
                    remarks = textEdit->toPlainText().trimmed();
                    qDebug() << "备注:" << remarks;
                }
            }
        }

        // 3. 验证必填字段
        if (customerId <= 0) {
            QMessageBox::warning(this, "错误", "请选择客户");
            return false;
        }
        if (productId <= 0) {
            QMessageBox::warning(this, "错误", "请选择商品");
            return false;
        }
        if (quantity <= 0) {
            QMessageBox::warning(this, "错误", "请输入有效的数量");
            return false;
        }
        if (unitPrice <= 0) {
            QMessageBox::warning(this, "错误", "请输入有效的单价");
            return false;
        }

        // 4. 插入订单表
        QSqlQuery orderQuery(m_db);
        orderQuery.prepare("INSERT INTO [order] (okey, ckey, [order status], date, oremarks) "
                           "VALUES (:okey, :ckey, :status, :date, :remarks)");
        orderQuery.bindValue(":okey", orderId);
        orderQuery.bindValue(":ckey", customerId);
        orderQuery.bindValue(":status", status);
        orderQuery.bindValue(":date", date);
        orderQuery.bindValue(":remarks", remarks);

        if (!orderQuery.exec()) {
            qDebug() << "❌ 订单表插入失败:" << orderQuery.lastError().text();
            QMessageBox::warning(this, "保存失败",
                                 "保存订单失败: " + orderQuery.lastError().text());
            return false;
        }

        qDebug() << "✅ 订单表插入成功";

        // 5. 获取odetails的odkey
        QSqlQuery odSeqQuery(m_db);
        if (!odSeqQuery.exec("SELECT NEXT VALUE FOR seq_order_detail_key")) {
            qDebug() << "获取odetails序列失败:" << odSeqQuery.lastError().text();
            // 回滚订单
            orderQuery.exec(QString("DELETE FROM [order] WHERE okey = %1").arg(orderId));
            return false;
        }

        int odetailsId = 1;
        if (odSeqQuery.next()) {
            odetailsId = odSeqQuery.value(0).toInt();
        }
        qDebug() << "生成的odetails ID:" << odetailsId;

        // 6. 插入odetails表
        QSqlQuery odQuery(m_db);
        orderQuery.prepare("INSERT INTO [order] (okey, ckey, [order status], date, oremarks) "
                           "VALUES (:okey, :ckey, :status, :date, :remarks)");
        odQuery.bindValue(":odkey", odetailsId);
        odQuery.bindValue(":okey", orderId);
        odQuery.bindValue(":pkey", productId);
        odQuery.bindValue(":quantity", quantity);
        odQuery.bindValue(":dprice", unitPrice);

        if (!odQuery.exec()) {
            qDebug() << "❌ odetails插入失败:" << odQuery.lastError().text();
            // 回滚订单
            orderQuery.exec(QString("DELETE FROM [order] WHERE okey = %1").arg(orderId));
            QMessageBox::warning(this, "保存失败",
                                 "保存订单详情失败: " + odQuery.lastError().text());
            return false;
        }

        qDebug() << "✅ 订单保存成功!";
        qDebug() << "  订单号:" << orderId;
        qDebug() << "  odetails ID:" << odetailsId;
        qDebug() << "  客户ID:" << customerId;
        qDebug() << "  商品ID:" << productId;
        qDebug() << "  数量:" << quantity;
        qDebug() << "  单价:" << unitPrice;
        qDebug() << "  总价:" << (quantity * unitPrice);

        return true;
    }
    else {  // 编辑模式
        qDebug() << "编辑模式未实现";
        return false;
    }
}
bool DataDialog::updateExistingOrder()
{
    qDebug() << "更新订单，订单号:" << m_keyValue;

    // 收集字段值
    int customerId = 0;
    QString status = "P";
    QDate date = QDate::currentDate();
    int productId = 0;
    int quantity = 0;
    double unitPrice = 0.0;
    QString remarks = "";

    for (QWidget* widget : m_inputWidgets) {
        QString fieldName = widget->objectName();

        if (fieldName == "ckey") {
            if (QComboBox* comboBox = qobject_cast<QComboBox*>(widget)) {
                customerId = comboBox->currentData().toInt();
            }
        }
        else if (fieldName == "order status") {
            if (QComboBox* comboBox = qobject_cast<QComboBox*>(widget)) {
                status = getStatusCode(comboBox->currentText());
            }
        }
        else if (fieldName == "date") {
            if (QDateEdit* dateEdit = qobject_cast<QDateEdit*>(widget)) {
                date = dateEdit->date();
            }
        }
        else if (fieldName == "pkey") {
            if (QComboBox* comboBox = qobject_cast<QComboBox*>(widget)) {
                productId = comboBox->currentData().toInt();
            }
        }
        else if (fieldName == "quantity") {
            if (QLineEdit* lineEdit = qobject_cast<QLineEdit*>(widget)) {
                quantity = lineEdit->text().toInt();
            }
        }
        else if (fieldName == "dprice") {
            if (QLineEdit* lineEdit = qobject_cast<QLineEdit*>(widget)) {
                unitPrice = lineEdit->text().toDouble();
            }
        }
        else if (fieldName == "oremarks") {
            if (QTextEdit* textEdit = qobject_cast<QTextEdit*>(widget)) {
                remarks = textEdit->toPlainText().trimmed();
            }
        }
    }

    // 验证
    if (customerId <= 0 || productId <= 0 || quantity <= 0 || unitPrice <= 0) {
        QMessageBox::warning(this, "错误", "请填写完整的订单信息");
        return false;
    }

    QSqlQuery query(m_db);

    // 1. 更新order表
    query.prepare("UPDATE [order] SET "
                  "ckey = :ckey, "
                  "[order status] = :status, "
                  "date = :date, "
                  "oremarks = :remarks "
                  "WHERE okey = :okey");

    query.bindValue(":ckey", customerId);
    query.bindValue(":status", status);
    query.bindValue(":date", date);
    query.bindValue(":remarks", remarks);
    query.bindValue(":okey", m_keyValue);

    if (!query.exec()) {
        qDebug() << "更新order表失败:" << query.lastError().text();
        QMessageBox::warning(this, "错误", "更新订单失败");
        return false;
    }

    // 2. 更新odetails表
    query.prepare("UPDATE odetails SET "
                  "pkey = :pkey, "
                  "quantity = :quantity, "
                  "dprice = :dprice "
                  "WHERE okey = :okey");

    query.bindValue(":pkey", productId);
    query.bindValue(":quantity", quantity);
    query.bindValue(":dprice", unitPrice);
    query.bindValue(":okey", m_keyValue);

    if (!query.exec()) {
        qDebug() << "更新odetails表失败:" << query.lastError().text();
        QMessageBox::warning(this, "错误", "更新订单详情失败");
        return false;
    }

    qDebug() << "订单更新成功";
    return true;
}
