#include "mainmenu.h"
#include "datadialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QListWidget>
#include <QListWidgetItem>
#include <QSplitter>
#include <QStackedWidget>
#include <QTableWidget>
#include <QHeaderView>
#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QInputDialog>
#include <QLineEdit>
#include <QTimer>
#include <functional>  // 用于std::function
#include <QColor>
#include <QApplication>
#include <QComboBox>
#include <QSet>




MainMenu::MainMenu(QWidget *parent, const QString &username, const QString &position, QSqlDatabase db)
    : QWidget(parent)
    , m_username(username)
    , m_position(position)
    , m_db(db)
    , m_splitter(nullptr)
    , m_menuList(nullptr)
    , m_contentStack(nullptr)
    , m_supplierTable(nullptr)
    , m_productTable(nullptr)
    , m_orderTable(nullptr)
    , m_customerTable(nullptr)
    , m_employeeTable(nullptr)
    , m_btnAddSupplier(nullptr)
    , m_btnDeleteSupplier(nullptr)
    , m_btnRefreshSupplier(nullptr)
    , m_btnAddProduct(nullptr)
    , m_btnDeleteProduct(nullptr)
    , m_btnRefreshProduct(nullptr)
    , m_btnAddOrder(nullptr)
    , m_btnDeleteOrder(nullptr)
    , m_btnRefreshOrder(nullptr)
    , m_btnAddCustomer(nullptr)
    , m_btnDeleteCustomer(nullptr)
    , m_btnRefreshCustomer(nullptr)
    , m_btnAddEmployee(nullptr)
    , m_btnDeleteEmployee(nullptr)
    , m_btnRefreshEmployee(nullptr)
{

    //
    QString cleanPosition = position.trimmed();

    // 2. 去除所有空白字符（包括制表符、换行符等）
    cleanPosition = cleanPosition.simplified();

    // 3. 如果还是太长，截取前几个字符
    if (cleanPosition.length() > 3) {
        cleanPosition = cleanPosition.left(3);
    }

    // 4. 规范化职位名称
    if (cleanPosition == "管理员") {
        m_position = "管理员";
    } else if (cleanPosition.toLower() == "admin" || cleanPosition.toLower() == "administrator") {
        m_position = "管理员";
    } else {
        m_position = cleanPosition;
    }
    setupDatabaseConnection();
    setupUI();
    setWindowTitle(QString("商贸管理系统 - %1 (%2)").arg(username).arg(position));

    resize(1000, 600);
    setMinimumSize(850, 500);
    setMaximumSize(1400, 800);
}

MainMenu::~MainMenu()
{
}

void MainMenu::setupDatabaseConnection()
{
    if (!m_db.isOpen()) {
        m_db = QSqlDatabase::addDatabase("QODBC", QString("connection_%1").arg((qulonglong)this));
        QString connectionString =
            "DRIVER={ODBC Driver 17 for SQL Server};"
            "SERVER=LAPTOP-23BEH08C;"
            "DATABASE=mgsystem;"
            "Trusted_Connection=yes;";

        m_db.setDatabaseName(connectionString);

        if (!m_db.open()) {
            QMessageBox::critical(this, "数据库错误",
                                  "数据库连接失败：" + m_db.lastError().text());
        }
    }
}

void MainMenu::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    QWidget *topBar = new QWidget(this);
    topBar->setFixedHeight(40);
    topBar->setStyleSheet("background-color: #1a5276;");

    QHBoxLayout *topLayout = new QHBoxLayout(topBar);
    topLayout->setContentsMargins(10, 0, 10, 0);
    topLayout->setSpacing(15);

    QLabel *titleLabel = new QLabel("商贸管理系统", topBar);
    titleLabel->setStyleSheet("color: white; font-size: 16px; font-weight: bold;");

    QLabel *userLabel = new QLabel(QString("当前用户: %1 | 职位: %2").arg(m_username).arg(m_position), topBar);
    userLabel->setStyleSheet("color: #e8f4fc; font-size: 12px;");

    QPushButton *logoutBtn = new QPushButton("退出登录", topBar);
    logoutBtn->setFixedSize(70, 25);
    logoutBtn->setStyleSheet(
        "QPushButton {"
        "background-color: #e74c3c;"
        "color: white;"
        "border: none;"
        "border-radius: 3px;"
        "font-size: 12px;"
        "}"
        "QPushButton:hover {"
        "background-color: #c0392b;"
        "}"
        );
    connect(logoutBtn, &QPushButton::clicked, this, &MainMenu::onLogoutClicked);

    topLayout->addWidget(titleLabel);
    topLayout->addStretch();
    topLayout->addWidget(userLabel);
    topLayout->addWidget(logoutBtn);

    m_splitter = new QSplitter(Qt::Horizontal, this);
    m_splitter->setHandleWidth(2);
    m_splitter->setStyleSheet("QSplitter::handle { background-color: #7fb3d5; }");

    m_menuList = new QListWidget(m_splitter);
    m_menuList->setFixedWidth(150);
    m_menuList->setStyleSheet(
        "QListWidget {"
        "background-color: #2c3e50;"
        "color: white;"
        "font-size: 13px;"
        "border: none;"
        "}"
        "QListWidget::item {"
        "height: 40px;"
        "padding-left: 15px;"
        "border-bottom: 1px solid #34495e;"
        "}"
        "QListWidget::item:selected {"
        "background-color: #3498db;"
        "border-left: 3px solid #2980b9;"
        "}"
        );
    // qDebug() << "=== 权限检查 ===";
    // qDebug() << "用户名:" << m_username;
    // qDebug() << "职位:" << m_position;
    // qDebug() << "职位长度:" << m_position.length();
    // qDebug() << "职位UTF-8:" << m_position.toUtf8().toHex();
    // qDebug() << "与'管理员'比较:" << (m_position == "管理员");
    // qDebug() << "与'管理员'比较(trimmed):" << (m_position.trimmed() == "管理员");

    QStringList menuItems = {"供应商管理", "商品管理", "订单管理", "顾客管理", "员工管理"};
    // 只有管理员能看见员工管理
    if (m_position == "管理员") {
        qDebug() << "当前是管理员，显示员工管理菜单";
        //menuItems.append("员工管理");
    } else {
        qDebug() << "当前是职员，不显示员工管理菜单。当前职位:" << m_position;
    }

    // 创建菜单列表
    for (int i = 0; i < menuItems.size(); i++) {
        QListWidgetItem *item = new QListWidgetItem(menuItems[i]);
        item->setData(Qt::UserRole, i);
        m_menuList->addItem(item);

    }

    //menuLayout->addWidget(m_menuList, 1);

    // for (const QString &item : menuItems) {
    //     QListWidgetItem *listItem = new QListWidgetItem(item, m_menuList);
    //     listItem->setSizeHint(QSize(140, 40));
    // }

    m_contentStack = new QStackedWidget(m_splitter);

    createSupplierPage();
    createProductPage();
    createOrderPage();
    createCustomerPage();
    //createEmployeePage();
    if (m_position == "管理员") {
        qDebug() << "创建员工管理页面";
        createEmployeePage();
    } else {
        qDebug() << "职员不创建员工管理页面";
        createNoPermissionPage();
    }

    m_splitter->addWidget(m_menuList);
    m_splitter->addWidget(m_contentStack);
    m_splitter->setStretchFactor(0, 1);
    m_splitter->setStretchFactor(1, 4);

    connect(m_menuList, &QListWidget::currentRowChanged, this, &MainMenu::onMenuClicked);

    mainLayout->addWidget(topBar);
    mainLayout->addWidget(m_splitter);

    m_menuList->setCurrentRow(1);
    refreshProductData();
}

void MainMenu::createSupplierPage()
{
    QWidget *page = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(page);
    layout->setContentsMargins(15, 15, 15, 15);
    layout->setSpacing(10);

    QLabel *title = new QLabel("供应商管理", page);
    title->setStyleSheet("font-size: 18px; font-weight: bold; color: #2c3e50;");

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(8);

    m_btnAddSupplier = new QPushButton("添加供应商", page);
    m_btnDeleteSupplier = new QPushButton("删除供应商", page);
    m_btnRefreshSupplier = new QPushButton("刷新", page);
    //

    // 设置对象名用于调试
    m_btnAddSupplier->setObjectName("btnAddSupplier");
    m_btnDeleteSupplier->setObjectName("btnDeleteSupplier");
    m_btnRefreshSupplier->setObjectName("btnRefreshSupplier");

    QString buttonStyle = "QPushButton {"
                          "padding: 6px 12px;"
                          "border-radius: 4px;"
                          "font-size: 12px;"
                          "min-width: 80px;"
                          "border: none;"
                          "}"
                          "QPushButton:hover {"
                          "opacity: 0.9;"
                          "}";

    m_btnAddSupplier->setStyleSheet(buttonStyle + "background-color: #2ecc71; color: white;");
    m_btnDeleteSupplier->setStyleSheet(buttonStyle + "background-color: #e74c3c; color: white;");
    m_btnRefreshSupplier->setStyleSheet(buttonStyle + "background-color: #3498db; color: white;");

    //
    m_btnRefreshSupplier->setStyleSheet(buttonStyle + "background-color: #3498db; color: white;");

    // 设置刷新按钮效果
    setupRefreshButton(m_btnRefreshSupplier, [this]() {
        refreshSupplierData();
    });

    // ✅ 正确的连接方式 - 连接到实际的槽函数
    connect(m_btnAddSupplier, &QPushButton::clicked, this, &MainMenu::onAddSupplierClicked);
    connect(m_btnDeleteSupplier, &QPushButton::clicked, this, &MainMenu::onDeleteSupplierClicked);
    connect(m_btnRefreshSupplier, &QPushButton::clicked, this, &MainMenu::refreshSupplierData);

    buttonLayout->addWidget(m_btnAddSupplier);
    buttonLayout->addWidget(m_btnDeleteSupplier);

    //
    createSearchWidget(page, buttonLayout, "搜索供应商名称或ID",
                       std::bind(&MainMenu::onSearchSupplier, this));

    buttonLayout->addStretch();
    buttonLayout->addWidget(m_btnRefreshSupplier);

    m_supplierTable = new QTableWidget(page);
    m_supplierTable->setColumnCount(2);
    m_supplierTable->setHorizontalHeaderLabels({"供应商ID", "供应商名称"});
    m_supplierTable->setAlternatingRowColors(true);
    m_supplierTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_supplierTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_supplierTable->verticalHeader()->setVisible(false);
    m_supplierTable->horizontalHeader()->setDefaultSectionSize(150);
    m_supplierTable->horizontalHeader()->setStretchLastSection(true);
    m_supplierTable->setStyleSheet(
        "QTableWidget {"
        "gridline-color: #bdc3c7;"
        "font-size: 12px;"
        "}"
        "QHeaderView::section {"
        "background-color: #3498db;"
        "color: white;"
        "padding: 8px;"
        "border: none;"
        "font-size: 12px;"
        "}"
        );

    layout->addWidget(title);
    layout->addLayout(buttonLayout);
    layout->addWidget(m_supplierTable, 1);

    m_contentStack->addWidget(page);

    connect(m_supplierTable, &QTableWidget::cellDoubleClicked,
            this, &MainMenu::onEditSupplierClicked);
}

void MainMenu::createProductPage()
{
    QWidget *page = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(page);
    layout->setContentsMargins(15, 15, 15, 15);
    layout->setSpacing(10);

    QLabel *title = new QLabel("商品管理", page);
    title->setStyleSheet("font-size: 18px; font-weight: bold; color: #2c3e50;");

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(8);

    m_btnAddProduct = new QPushButton("添加商品", page);
    m_btnDeleteProduct = new QPushButton("删除商品", page);
    m_btnRefreshProduct = new QPushButton("刷新", page);

    QString buttonStyle = "QPushButton {"
                          "padding: 6px 12px;"
                          "border-radius: 4px;"
                          "font-size: 12px;"
                          "min-width: 80px;"
                          "border: none;"
                          "}"
                          "QPushButton:hover {"
                          "opacity: 0.9;"
                          "}";

    m_btnAddProduct->setStyleSheet(buttonStyle + "background-color: #2ecc71; color: white;");
    m_btnDeleteProduct->setStyleSheet(buttonStyle + "background-color: #e74c3c; color: white;");
    m_btnRefreshProduct->setStyleSheet(buttonStyle + "background-color: #3498db; color: white;");

    //

    m_btnRefreshProduct->setStyleSheet(buttonStyle + "background-color: #3498db; color: white;");

    // 设置刷新按钮效果
    setupRefreshButton(m_btnRefreshProduct, [this]() {
        refreshProductData();
    });

    // ✅ 正确的连接
    connect(m_btnAddProduct, &QPushButton::clicked, this, &MainMenu::onAddProductClicked);
    connect(m_btnDeleteProduct, &QPushButton::clicked, this, &MainMenu::onDeleteProductClicked);
    connect(m_btnRefreshProduct, &QPushButton::clicked, this, &MainMenu::refreshProductData);
    // connect(m_btnRefreshProduct, &QPushButton::clicked, this, [this]() {
    //     //qDebug() << "刷新按钮被点击，测试QTimer";

    //     // 1. 保存原始样式
    //     QString originalStyle = m_btnRefreshProduct->styleSheet();

    //     // 2. 设置点击时的深蓝色
    //     m_btnRefreshProduct->setStyleSheet(
    //         "QPushButton {"
    //         "padding: 6px 12px;"
    //         "border-radius: 4px;"
    //         "font-size: 12px;"
    //         "min-width: 80px;"
    //         "border: none;"
    //         "background-color: #E0EDFE;"  // 深蓝色
    //         "color: white;"
    //         "}"
    //         );

    //     // 3. 强制界面更新
    //     m_btnRefreshProduct->update();

    //     // 4. 使用QTimer恢复颜色
    //     QTimer::singleShot(200, this, [this, originalStyle]() {
    //         //qDebug() << "QTimer触发，恢复按钮颜色";
    //         m_btnRefreshProduct->setStyleSheet(originalStyle);
    //         m_btnRefreshProduct->update();
    //     });

    //     // 5. 刷新数据
    //     refreshProductData();
    // });

    buttonLayout->addWidget(m_btnAddProduct);
    buttonLayout->addWidget(m_btnDeleteProduct);
    createSearchWidget(page, buttonLayout, "搜索商品名称或ID",
                         std::bind(&MainMenu::onSearchProduct, this));
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_btnRefreshProduct);

    m_productTable = new QTableWidget(page);
    m_productTable->setColumnCount(7);
    m_productTable->setHorizontalHeaderLabels({"商品ID", "名称", "类型", "售价", "库存", "已售", "备注"});
    m_productTable->setAlternatingRowColors(true);
    m_productTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_productTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_productTable->verticalHeader()->setVisible(false);
    m_productTable->verticalHeader()->setDefaultSectionSize(30);
    m_productTable->horizontalHeader()->setDefaultSectionSize(100);
    m_productTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    m_productTable->horizontalHeader()->setSectionResizeMode(6, QHeaderView::Stretch);
    m_productTable->setStyleSheet(
        "QTableWidget {"
        "gridline-color: #bdc3c7;"
        "font-size: 12px;"
        "}"
        "QHeaderView::section {"
        "background-color: #3498db;"
        "color: white;"
        "padding: 8px;"
        "border: none;"
        "font-size: 12px;"
        "}"
        );

    layout->addWidget(title);
    layout->addLayout(buttonLayout);
    layout->addWidget(m_productTable, 1);

    m_contentStack->addWidget(page);

    connect(m_productTable, &QTableWidget::cellDoubleClicked,
            this, &MainMenu::onEditProductClicked);
    connect(m_btnRefreshProduct, &QPushButton::clicked,
            this, &MainMenu::refreshProductData);
}

void MainMenu::createOrderPage()
{
    QWidget *page = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(page);
    layout->setContentsMargins(15, 15, 15, 15);
    layout->setSpacing(10);

    QLabel *title = new QLabel("订单管理", page);
    title->setStyleSheet("font-size: 18px; font-weight: bold; color: #2c3e50;");

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(8);

    m_btnAddOrder = new QPushButton("添加订单", page);
    m_btnDeleteOrder = new QPushButton("删除订单", page);
    m_btnRefreshOrder = new QPushButton("刷新", page);

    QString buttonStyle = "QPushButton {"
                          "padding: 6px 12px;"
                          "border-radius: 4px;"
                          "font-size: 12px;"
                          "min-width: 80px;"
                          "border: none;"
                          "}"
                          "QPushButton:hover {"
                          "opacity: 0.9;"
                          "}";

    m_btnAddOrder->setStyleSheet(buttonStyle + "background-color: #2ecc71; color: white;");
    m_btnDeleteOrder->setStyleSheet(buttonStyle + "background-color: #e74c3c; color: white;");
    m_btnRefreshOrder->setStyleSheet(buttonStyle + "background-color: #3498db; color: white;");

    //

    m_btnRefreshOrder->setStyleSheet(buttonStyle + "background-color: #3498db; color: white;");

    // 设置刷新按钮效果
    setupRefreshButton(m_btnRefreshOrder, [this]() {
        refreshOrderData();
    });

    // ✅ 正确的连接
    connect(m_btnAddOrder, &QPushButton::clicked, this, &MainMenu::onAddOrderClicked);
    connect(m_btnDeleteOrder, &QPushButton::clicked, this, &MainMenu::onDeleteOrderClicked);
    connect(m_btnRefreshOrder, &QPushButton::clicked, this, &MainMenu::refreshOrderData);

    buttonLayout->addWidget(m_btnAddOrder);
    buttonLayout->addWidget(m_btnDeleteOrder);
    createSearchWidget(page, buttonLayout, "搜索订单号、客户ID或状态",
                       std::bind(&MainMenu::onSearchOrder, this));
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_btnRefreshOrder);

    m_orderTable = new QTableWidget(page);
    m_orderTable->setColumnCount(8);
    m_orderTable->setHorizontalHeaderLabels({"订单号", "客户ID", "订单状态", "日期", "备注",
                                             "商品ID", "数量", "成交单价"});
    m_orderTable->setAlternatingRowColors(true);
    m_orderTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_orderTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_orderTable->verticalHeader()->setVisible(false);
    m_orderTable->verticalHeader()->setDefaultSectionSize(30);
    m_orderTable->horizontalHeader()->setDefaultSectionSize(100);
    m_orderTable->horizontalHeader()->setSectionResizeMode(4, QHeaderView::Stretch);
    m_orderTable->setStyleSheet(
        "QTableWidget {"
        "gridline-color: #bdc3c7;"
        "font-size: 12px;"
        "}"
        "QHeaderView::section {"
        "background-color: #3498db;"
        "color: white;"
        "padding: 8px;"
        "border: none;"
        "font-size: 12px;"
        "}"
        );

    layout->addWidget(title);
    layout->addLayout(buttonLayout);
    layout->addWidget(m_orderTable, 1);

    m_contentStack->addWidget(page);
    connect(m_orderTable, &QTableWidget::cellDoubleClicked,
            this, &MainMenu::onEditOrderClicked);
}

void MainMenu::createCustomerPage()
{
    QWidget *page = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(page);
    layout->setContentsMargins(15, 15, 15, 15);
    layout->setSpacing(10);

    QLabel *title = new QLabel("顾客管理", page);
    title->setStyleSheet("font-size: 18px; font-weight: bold; color: #2c3e50;");

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(8);

    m_btnAddCustomer = new QPushButton("添加顾客", page);
    m_btnDeleteCustomer = new QPushButton("删除顾客", page);
    m_btnRefreshCustomer = new QPushButton("刷新", page);

    QString buttonStyle = "QPushButton {"
                          "padding: 6px 12px;"
                          "border-radius: 4px;"
                          "font-size: 12px;"
                          "min-width: 80px;"
                          "border: none;"
                          "}"
                          "QPushButton:hover {"
                          "opacity: 0.9;"
                          "}";

    m_btnAddCustomer->setStyleSheet(buttonStyle + "background-color: #2ecc71; color: white;");
    m_btnDeleteCustomer->setStyleSheet(buttonStyle + "background-color: #e74c3c; color: white;");
    m_btnRefreshCustomer->setStyleSheet(buttonStyle + "background-color: #3498db; color: white;");

    //


    m_btnRefreshCustomer->setStyleSheet(buttonStyle + "background-color: #3498db; color: white;");

    // 设置刷新按钮效果
    setupRefreshButton(m_btnRefreshCustomer, [this]() {
        refreshCustomerData();
    });

    // ✅ 正确的连接
    connect(m_btnAddCustomer, &QPushButton::clicked, this, &MainMenu::onAddCustomerClicked);
    connect(m_btnDeleteCustomer, &QPushButton::clicked, this, &MainMenu::onDeleteCustomerClicked);
    connect(m_btnRefreshCustomer, &QPushButton::clicked, this, &MainMenu::refreshCustomerData);

    buttonLayout->addWidget(m_btnAddCustomer);
    buttonLayout->addWidget(m_btnDeleteCustomer);
    createSearchWidget(page, buttonLayout, "搜索顾客姓名或ID",
                       std::bind(&MainMenu::onSearchCustomer, this));
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_btnRefreshCustomer);

    m_customerTable = new QTableWidget(page);
    m_customerTable->setColumnCount(3);
    m_customerTable->setHorizontalHeaderLabels({"顾客ID", "姓名", "备注"});
    m_customerTable->setAlternatingRowColors(true);
    m_customerTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_customerTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_customerTable->verticalHeader()->setVisible(false);
    m_customerTable->verticalHeader()->setDefaultSectionSize(30);
    m_customerTable->horizontalHeader()->setDefaultSectionSize(150);
    m_customerTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    m_customerTable->setStyleSheet(
        "QTableWidget {"
        "gridline-color: #bdc3c7;"
        "font-size: 12px;"
        "}"
        "QHeaderView::section {"
        "background-color: #3498db;"
        "color: white;"
        "padding: 8px;"
        "border: none;"
        "font-size: 12px;"
        "}"
        );

    layout->addWidget(title);
    layout->addLayout(buttonLayout);
    layout->addWidget(m_customerTable, 1);

    m_contentStack->addWidget(page);
    connect(m_customerTable, &QTableWidget::cellDoubleClicked,
            this, &MainMenu::onEditCustomerClicked);
}

void MainMenu::createEmployeePage()
{
    QWidget *page = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(page);
    layout->setContentsMargins(15, 15, 15, 15);
    layout->setSpacing(10);

    QLabel *title = new QLabel("员工管理", page);
    title->setStyleSheet("font-size: 18px; font-weight: bold; color: #2c3e50;");

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(8);

    m_btnAddEmployee = new QPushButton("添加员工", page);
    m_btnDeleteEmployee = new QPushButton("删除员工", page);
    m_btnRefreshEmployee = new QPushButton("刷新", page);

    QString buttonStyle = "QPushButton {"
                          "padding: 6px 12px;"
                          "border-radius: 4px;"
                          "font-size: 12px;"
                          "min-width: 80px;"
                          "border: none;"
                          "}"
                          "QPushButton:hover {"
                          "opacity: 0.9;"
                          "}";

    m_btnAddEmployee->setStyleSheet(buttonStyle + "background-color: #2ecc71; color: white;");
    m_btnDeleteEmployee->setStyleSheet(buttonStyle + "background-color: #e74c3c; color: white;");
    m_btnRefreshEmployee->setStyleSheet(buttonStyle + "background-color: #3498db; color: white;");

    //

    m_btnRefreshEmployee->setStyleSheet(buttonStyle + "background-color: #3498db; color: white;");

    // 设置刷新按钮效果
    setupRefreshButton(m_btnRefreshEmployee, [this]() {
        refreshEmployeeData();
    });

    // ✅ 正确的连接
    connect(m_btnAddEmployee, &QPushButton::clicked, this, &MainMenu::onAddEmployeeClicked);
    connect(m_btnDeleteEmployee, &QPushButton::clicked, this, &MainMenu::onDeleteEmployeeClicked);
    connect(m_btnRefreshEmployee, &QPushButton::clicked, this, &MainMenu::refreshEmployeeData);

    buttonLayout->addWidget(m_btnAddEmployee);
    buttonLayout->addWidget(m_btnDeleteEmployee);
    createSearchWidget(page, buttonLayout, "搜索员工姓名、ID或职位",
                       std::bind(&MainMenu::onSearchEmployee, this));
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_btnRefreshEmployee);

    m_employeeTable = new QTableWidget(page);
    m_employeeTable->setColumnCount(4);
    m_employeeTable->setHorizontalHeaderLabels({"员工号", "姓名", "职位", "密码"});
    m_employeeTable->setAlternatingRowColors(true);
    m_employeeTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_employeeTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_employeeTable->verticalHeader()->setVisible(false);
    m_employeeTable->verticalHeader()->setDefaultSectionSize(30);
    m_employeeTable->horizontalHeader()->setDefaultSectionSize(120);
    m_employeeTable->horizontalHeader()->setStretchLastSection(true);
    m_employeeTable->setStyleSheet(
        "QTableWidget {"
        "gridline-color: #bdc3c7;"
        "font-size: 12px;"
        "}"
        "QHeaderView::section {"
        "background-color: #3498db;"
        "color: white;"
        "padding: 8px;"
        "border: none;"
        "font-size: 12px;"
        "}"
        );

    layout->addWidget(title);
    layout->addLayout(buttonLayout);
    layout->addWidget(m_employeeTable, 1);

    m_contentStack->addWidget(page);
    connect(m_employeeTable, &QTableWidget::cellDoubleClicked,
            this, &MainMenu::onEditEmployeeClicked);
}

// 刷新数据函数
void MainMenu::refreshSupplierData()
{
    if (!m_db.isOpen()) {
        QMessageBox::warning(this, "错误", "数据库未连接");
        return;
    }

    QSqlQuery query(m_db);
    query.prepare("SELECT skey, sname FROM supplier ORDER BY skey");

    if (!query.exec()) {
        QMessageBox::warning(this, "查询错误", "查询供应商数据失败: " + query.lastError().text());
        return;
    }

    m_supplierTable->setRowCount(0);
    int row = 0;

    while (query.next()) {
        m_supplierTable->insertRow(row);
        m_supplierTable->setItem(row, 0, new QTableWidgetItem(query.value(0).toString()));
        m_supplierTable->setItem(row, 1, new QTableWidgetItem(query.value(1).toString()));
        row++;
    }
    //
    // 添加统计信息
    m_supplierTable->setItem(row, 0, new QTableWidgetItem("总计:"));
    m_supplierTable->setItem(row, 1, new QTableWidgetItem(QString::number(row) + " 个供应商"));
}

void MainMenu::refreshProductData()
{
    //int oldRowCount = m_productTable->rowCount();
    if (!m_db.isOpen()) {
        QMessageBox::warning(this, "错误", "数据库未连接");
        return;
    }

    QSqlQuery query(m_db);
    query.prepare("SELECT pkey, pname, type, sprice, stock, sold, premarks FROM product ORDER BY pkey");

    if (!query.exec()) {
        QMessageBox::warning(this, "查询错误", "查询商品数据失败: " + query.lastError().text());
        return;
    }

    m_productTable->setRowCount(0);
    int row = 0;

    while (query.next()) {
        m_productTable->insertRow(row);
        for (int col = 0; col < 7; col++) {
            QString value = query.value(col).toString();
            if (col == 3) {
                value = QString("¥%1").arg(query.value(col).toDouble());
            }
            m_productTable->setItem(row, col, new QTableWidgetItem(value));
        }
        row++;
    }
}
    //
    // int totalStock = 0;
    // int totalSold = 0;
    // double totalSales = 0;

    // for (int i = 0; i < row; i++) {
    //     int stock = m_productTable->item(i, 4)->text().toInt();
    //     int sold = m_productTable->item(i, 5)->text().toInt();
    //     QString priceStr = m_productTable->item(i, 3)->text();
    //     priceStr = priceStr.mid(1); // 去掉¥符号
    //     double price = priceStr.toDouble();

    //     totalStock += stock;
    //     totalSold += sold;
    //     totalSales += sold * price;
    // }

    // // 添加统计行
    // m_productTable->insertRow(row);
    // m_productTable->setItem(row, 0, new QTableWidgetItem("总计:"));
    // m_productTable->setItem(row, 1, new QTableWidgetItem(QString::number(row) + " 种商品"));
    // m_productTable->setItem(row, 2, new QTableWidgetItem("统计"));
    // m_productTable->setItem(row, 3, new QTableWidgetItem("-"));
    // m_productTable->setItem(row, 4, new QTableWidgetItem(QString::number(totalStock)));
    // m_productTable->setItem(row, 5, new QTableWidgetItem(QString::number(totalSold)));
    // m_productTable->setItem(row, 6, new QTableWidgetItem(QString("销售额: ¥%1").arg(totalSales, 0, 'f', 2)));

    // // 设置统计行样式
    // for (int col = 0; col < 7; col++) {
    //     QTableWidgetItem *item = m_productTable->item(row, col);
    //     if (item) {
    //         item->setBackground(QColor(240, 240, 240));
    //         item->setForeground(QColor(0, 0, 0));
    //         if (col == 6) { // 备注列
    //             item->setForeground(QColor(0, 128, 0)); // 绿色
    //         }
    //     }
    // }
    // //qDebug() << "商品数据刷新完成，共" << row << "条记录";
// }

void MainMenu::refreshOrderData()
{
    if (!m_db.isOpen()) {
        QMessageBox::warning(this, "错误", "数据库未连接");
        return;
    }

    QSqlQuery query(m_db);
    query.prepare("SELECT o.okey, o.ckey, o.[order status], o.date, o.oremarks, "
                  "od.pkey, od.quantity, od.dprice "
                  "FROM [order] o "
                  "JOIN odetails od ON o.okey = od.okey "
                  "ORDER BY o.date DESC");

    if (!query.exec()) {
        QMessageBox::warning(this, "查询错误", "查询订单数据失败: " + query.lastError().text());
        return;
    }

    m_orderTable->setRowCount(0);
    int row = 0;

    while (query.next()) {
        m_orderTable->insertRow(row);
        for (int col = 0; col < 8; col++) {
            QString value = query.value(col).toString();
            if (col == 2) {
                if (value == "F") value = "已完成";
                else if (value == "P") value = "进行中";
                else if (value == "E") value = "已取消";
            } else if (col == 7) {
                value = QString("¥%1").arg(query.value(col).toDouble());
            }
            m_orderTable->setItem(row, col, new QTableWidgetItem(value));
        }
        row++;
    }
}

void MainMenu::refreshCustomerData()
{
    if (!m_db.isOpen()) {
        QMessageBox::warning(this, "错误", "数据库未连接");
        return;
    }

    QSqlQuery query(m_db);
    query.prepare("SELECT ckey, cname, cremarks FROM customer ORDER BY ckey");

    if (!query.exec()) {
        QMessageBox::warning(this, "查询错误", "查询顾客数据失败: " + query.lastError().text());
        return;
    }

    m_customerTable->setRowCount(0);
    int row = 0;

    while (query.next()) {
        m_customerTable->insertRow(row);
        for (int col = 0; col < 3; col++) {
            m_customerTable->setItem(row, col, new QTableWidgetItem(query.value(col).toString()));
        }
        row++;
    }
}

void MainMenu::refreshEmployeeData()
{
    if (!m_db.isOpen()) {
        QMessageBox::warning(this, "错误", "数据库未连接");
        return;
    }

    QSqlQuery query(m_db);
    query.prepare("SELECT ekey, ename, position, password FROM employee ORDER BY ekey");

    if (!query.exec()) {
        QMessageBox::warning(this, "查询错误", "查询员工数据失败: " + query.lastError().text());
        return;
    }

    m_employeeTable->setRowCount(0);
    int row = 0;

    while (query.next()) {
        m_employeeTable->insertRow(row);
        for (int col = 0; col < 4; col++) {
            QString value = query.value(col).toString();
            if (col == 3 && m_position != "管理员") {
                value = "***";
            }
            m_employeeTable->setItem(row, col, new QTableWidgetItem(value));
        }
        row++;
    }
}

// 添加功能的实现
void MainMenu::onAddSupplierClicked()
{
    qDebug() << "添加供应商按钮被点击";

    if (!m_db.isOpen()) {
        QMessageBox::warning(this, "错误", "数据库未连接");
        return;
    }

    try {
        DataDialog dialog(m_db, DataDialog::SupplierDialog, DataDialog::Add, this);

        if (dialog.exec() == QDialog::Accepted) {
            refreshSupplierData();
        }
    } catch (const std::exception& e) {
        QMessageBox::critical(this, "错误", QString("对话框创建失败: %1").arg(e.what()));
    }
}

// void MainMenu::refreshProductData()
// {
//     if (!m_db.isOpen()) {
//         QMessageBox::warning(this, "错误", "数据库未连接");
//         return;
//     }

//     QSqlQuery query(m_db);
//     query.prepare("SELECT pkey, pname, type, sprice, stock, sold, premarks FROM product ORDER BY pkey");

//     if (!query.exec()) {
//         QMessageBox::warning(this, "查询错误", "查询商品数据失败: " + query.lastError().text());
//         return;
//     }

//     m_productTable->setRowCount(0);

//     // 清空并重新加载类型筛选框
//     if (m_productTypeFilter) {
//         m_productTypeFilter->clear();
//         m_productTypeFilter->addItem("全部类型");

//         QSet<QString> uniqueTypes;
//         while (query.next()) {
//             QString type = query.value(2).toString();
//             if (!type.isEmpty()) {
//                 uniqueTypes.insert(type);
//             }
//         }

//         // 重新执行查询
//         query.finish();
//         query.exec("SELECT pkey, pname, type, sprice, stock, sold, premarks FROM product ORDER BY pkey");

//         // 添加唯一类型到下拉框
//         QStringList typeList = uniqueTypes.values();
//         typeList.sort();
//         m_productTypeFilter->addItems(typeList);
//     }

//     int row = 0;
//     while (query.next()) {
//         m_productTable->insertRow(row);
//         for (int col = 0; col < 7; col++) {
//             QString value = query.value(col).toString();
//             if (col == 3) {
//                 value = QString("¥%1").arg(query.value(col).toDouble());
//             }
//             m_productTable->setItem(row, col, new QTableWidgetItem(value));
//         }
//         row++;
//     }
// }

void MainMenu::onAddCustomerClicked()
{
    qDebug() << "添加顾客按钮被点击";

    if (!m_db.isOpen()) {
        QMessageBox::warning(this, "错误", "数据库未连接");
        return;
    }

    try {
        DataDialog dialog(m_db, DataDialog::CustomerDialog, DataDialog::Add, this);

        if (dialog.exec() == QDialog::Accepted) {
            refreshCustomerData();
        }
    } catch (const std::exception& e) {
        QMessageBox::critical(this, "错误", QString("对话框创建失败: %1").arg(e.what()));
    }
}

void MainMenu::onAddEmployeeClicked()
{
    qDebug() << "添加员工按钮被点击";

    if (!m_db.isOpen()) {
        QMessageBox::warning(this, "错误", "数据库未连接");
        return;
    }

    try {
        DataDialog dialog(m_db, DataDialog::EmployeeDialog, DataDialog::Add, this);

        if (dialog.exec() == QDialog::Accepted) {
            refreshEmployeeData();
        }
    } catch (const std::exception& e) {
        QMessageBox::critical(this, "错误", QString("对话框创建失败: %1").arg(e.what()));
    }
}

void MainMenu::onAddOrderClicked()
{
    qDebug() << "添加订单按钮被点击";

    // 检查数据库连接
    if (!m_db.isOpen()) {
        QMessageBox::warning(this, "错误", "数据库未连接");
        return;
    }

    qDebug() << "创建DataDialog订单对话框";

    try {
        // 使用您创建的DataDialog
        DataDialog dialog(m_db, DataDialog::OrderDialog, DataDialog::Add, this);

        // 显示对话框
        int result = dialog.exec();
        qDebug() << "DataDialog结果:" << result;

        if (result == QDialog::Accepted) {
            qDebug() << "DataDialog被接受，刷新订单数据";
            refreshOrderData();
        } else {
            qDebug() << "DataDialog被取消";
        }
    } catch (const std::exception& e) {
        qDebug() << "DataDialog创建失败:" << e.what();
        QMessageBox::critical(this, "错误", QString("对话框创建失败: %1").arg(e.what()));
    }
}
// mainmenu.cpp
void MainMenu::onAddProductClicked()
{
    qDebug() << "添加商品按钮被点击";

    if (!m_db.isOpen()) {
        QMessageBox::warning(this, "错误", "数据库未连接");
        return;
    }

    try {
        DataDialog dialog(m_db, DataDialog::ProductDialog, DataDialog::Add, this);

        if (dialog.exec() == QDialog::Accepted) {
            refreshProductData();
        }
    } catch (const std::exception& e) {
        QMessageBox::critical(this, "错误", QString("对话框创建失败: %1").arg(e.what()));
    }
}

// 删除功能的实现
void MainMenu::onDeleteSupplierClicked()
{
    int currentRow = m_supplierTable->currentRow();
    if (currentRow < 0) {
        QMessageBox::warning(this, "提示", "请先选择要删除的供应商");
        return;
    }

    QString skey = m_supplierTable->item(currentRow, 0)->text();
    QString sname = m_supplierTable->item(currentRow, 1)->text();

    int ret = QMessageBox::question(this, "确认删除",
                                    QString("确定要删除供应商 '%1' 吗？").arg(sname),
                                    QMessageBox::Yes | QMessageBox::No);

    if (ret == QMessageBox::Yes) {
        QSqlQuery query(m_db);
        query.prepare("DELETE FROM supplier WHERE skey = :skey");
        query.bindValue(":skey", skey.toInt());

        if (query.exec()) {
            if (query.numRowsAffected() > 0) {
                QMessageBox::information(this, "成功", "供应商删除成功！");
                refreshSupplierData();
            }
        } else {
            QMessageBox::warning(this, "错误", "删除失败: " + query.lastError().text());
        }
    }
}

void MainMenu::onDeleteProductClicked()
{
    int currentRow = m_productTable->currentRow();
    if (currentRow < 0) {
        QMessageBox::warning(this, "提示", "请先选择要删除的商品");
        return;
    }

    QString pkey = m_productTable->item(currentRow, 0)->text();
    QString pname = m_productTable->item(currentRow, 1)->text();

    int ret = QMessageBox::question(this, "确认删除",
                                    QString("确定要删除商品 '%1' 吗？").arg(pname),
                                    QMessageBox::Yes | QMessageBox::No);

    if (ret == QMessageBox::Yes) {
        QSqlQuery query(m_db);
        query.prepare("DELETE FROM product WHERE pkey = :pkey");
        query.bindValue(":pkey", pkey.toInt());

        if (query.exec()) {
            if (query.numRowsAffected() > 0) {
                QMessageBox::information(this, "成功", "商品删除成功！");
                refreshProductData();
            }
        } else {
            QMessageBox::warning(this, "错误", "删除失败: " + query.lastError().text());
        }
    }
}

void MainMenu::onDeleteOrderClicked()
{
    int currentRow = m_orderTable->currentRow();
    if (currentRow < 0) {
        QMessageBox::warning(this, "提示", "请先选择要删除的订单");
        return;
    }

    QString okey = m_orderTable->item(currentRow, 0)->text();

    int ret = QMessageBox::question(this, "确认删除",
                                    QString("确定要删除订单 '%1' 吗？").arg(okey),
                                    QMessageBox::Yes | QMessageBox::No);

    if (ret == QMessageBox::Yes) {
        QSqlQuery query(m_db);
        query.prepare("DELETE FROM [order] WHERE okey = :okey");
        query.bindValue(":okey", okey.toInt());

        if (query.exec()) {
            if (query.numRowsAffected() > 0) {
                QMessageBox::information(this, "成功", "订单删除成功！");
                refreshOrderData();
            }
        } else {
            QMessageBox::warning(this, "错误", "删除失败: " + query.lastError().text());
        }
    }
}

void MainMenu::onDeleteCustomerClicked()
{
    int currentRow = m_customerTable->currentRow();
    if (currentRow < 0) {
        QMessageBox::warning(this, "提示", "请先选择要删除的顾客");
        return;
    }

    QString ckey = m_customerTable->item(currentRow, 0)->text();
    QString cname = m_customerTable->item(currentRow, 1)->text();

    int ret = QMessageBox::question(this, "确认删除",
                                    QString("确定要删除顾客 '%1' 吗？").arg(cname),
                                    QMessageBox::Yes | QMessageBox::No);

    if (ret == QMessageBox::Yes) {
        QSqlQuery query(m_db);
        query.prepare("DELETE FROM customer WHERE ckey = :ckey");
        query.bindValue(":ckey", ckey.toInt());

        if (query.exec()) {
            if (query.numRowsAffected() > 0) {
                QMessageBox::information(this, "成功", "顾客删除成功！");
                refreshCustomerData();
            }
        } else {
            QMessageBox::warning(this, "错误", "删除失败: " + query.lastError().text());
        }
    }
}

void MainMenu::onDeleteEmployeeClicked()
{
    int currentRow = m_employeeTable->currentRow();
    if (currentRow < 0) {
        QMessageBox::warning(this, "提示", "请先选择要删除的员工");
        return;
    }

    QString ekey = m_employeeTable->item(currentRow, 0)->text();
    QString ename = m_employeeTable->item(currentRow, 1)->text();

    int ret = QMessageBox::question(this, "确认删除",
                                    QString("确定要删除员工 '%1' 吗？").arg(ename),
                                    QMessageBox::Yes | QMessageBox::No);

    if (ret == QMessageBox::Yes) {
        QSqlQuery query(m_db);
        query.prepare("DELETE FROM employee WHERE ekey = :ekey");
        query.bindValue(":ekey", ekey.toInt());

        if (query.exec()) {
            if (query.numRowsAffected() > 0) {
                QMessageBox::information(this, "成功", "员工删除成功！");
                refreshEmployeeData();
            }
        } else {
            QMessageBox::warning(this, "错误", "删除失败: " + query.lastError().text());
        }
    }
}

// 其他功能
// void MainMenu::onMenuClicked(int index)
// {
//     m_contentStack->setCurrentIndex(index);

//     switch (index) {
//     case 0: refreshSupplierData(); break;
//     case 1: refreshProductData(); break;
//     case 2: refreshOrderData(); break;
//     case 3: refreshCustomerData(); break;
//     case 4: refreshEmployeeData(); break;
//     }
// }
void MainMenu::onMenuClicked(int index)
{
    if (index < 0) return;

    QListWidgetItem *item = m_menuList->item(index);
    if (!item) return;

    QString menuText = item->text();
    qDebug() << "点击菜单:" << menuText << "索引:" << index;

    m_contentStack->setCurrentIndex(index);
    // 如果是员工管理页面
    if (menuText == "员工管理") {
        if (m_position == "管理员") {
            m_contentStack->setCurrentIndex(index);
            refreshEmployeeData();
        } else {
            // 职员仍然显示员工管理页面（但显示无权限内容）
            m_contentStack->setCurrentIndex(index);
        }
        return;
    }

    // 其他页面正常处理
    m_contentStack->setCurrentIndex(index);

    switch (index) {
    case 0:
        refreshSupplierData();
        break;
    case 1:
        refreshProductData();
        break;
    case 2:
        refreshOrderData();
        break;
    case 3:
        refreshCustomerData();
        break;
    }
}
void MainMenu::onLogoutClicked()
{
    emit windowClosed();
    close();
}

// 双击编辑供应商
void MainMenu::onEditSupplierClicked()
{
    qDebug() << "双击编辑供应商";

    int currentRow = m_supplierTable->currentRow();
    if (currentRow < 0) {
        QMessageBox::warning(this, "提示", "请双击选择要编辑的供应商");
        return;
    }

    int skey = m_supplierTable->item(currentRow, 0)->text().toInt();
    QString sname = m_supplierTable->item(currentRow, 1)->text();

    qDebug() << "编辑供应商 ID:" << skey << "名称:" << sname;

    // 使用您现有的DataDialog
    DataDialog dialog(m_db, DataDialog::SupplierDialog, DataDialog::Edit, skey, this);

    if (dialog.exec() == QDialog::Accepted) {
        refreshSupplierData();
        QMessageBox::information(this, "成功", "供应商信息已更新");
    }
}

// 双击编辑商品
void MainMenu::onEditProductClicked()
{
    qDebug() << "双击编辑商品";

    int currentRow = m_productTable->currentRow();
    if (currentRow < 0) {
        QMessageBox::warning(this, "提示", "请双击选择要编辑的商品");
        return;
    }

    int pkey = m_productTable->item(currentRow, 0)->text().toInt();
    QString pname = m_productTable->item(currentRow, 1)->text();

    qDebug() << "编辑商品 ID:" << pkey << "名称:" << pname;

    DataDialog dialog(m_db, DataDialog::ProductDialog, DataDialog::Edit, pkey, this);

    if (dialog.exec() == QDialog::Accepted) {
        refreshProductData();
        QMessageBox::information(this, "成功", "商品信息已更新");
    }
}

// 双击编辑订单
void MainMenu::onEditOrderClicked()
{
    qDebug() << "双击编辑订单";

    int currentRow = m_orderTable->currentRow();
    if (currentRow < 0) {
        QMessageBox::warning(this, "提示", "请双击选择要编辑的订单");
        return;
    }

    int okey = m_orderTable->item(currentRow, 0)->text().toInt();
    QString ckey = m_orderTable->item(currentRow, 1)->text();

    qDebug() << "编辑订单 ID:" << okey << "客户ID:" << ckey;

    DataDialog dialog(m_db, DataDialog::OrderDialog, DataDialog::Edit, okey, this);

    if (dialog.exec() == QDialog::Accepted) {
        refreshOrderData();
        QMessageBox::information(this, "成功", "订单信息已更新");
    }
}

// 双击编辑顾客
void MainMenu::onEditCustomerClicked()
{
    qDebug() << "双击编辑顾客";

    int currentRow = m_customerTable->currentRow();
    if (currentRow < 0) {
        QMessageBox::warning(this, "提示", "请双击选择要编辑的顾客");
        return;
    }

    int ckey = m_customerTable->item(currentRow, 0)->text().toInt();
    QString cname = m_customerTable->item(currentRow, 1)->text();

    qDebug() << "编辑顾客 ID:" << ckey << "姓名:" << cname;

    DataDialog dialog(m_db, DataDialog::CustomerDialog, DataDialog::Edit, ckey, this);

    if (dialog.exec() == QDialog::Accepted) {
        refreshCustomerData();
        QMessageBox::information(this, "成功", "顾客信息已更新");
    }
}

// 双击编辑员工
void MainMenu::onEditEmployeeClicked()
{
    qDebug() << "双击编辑员工";

    int currentRow = m_employeeTable->currentRow();
    if (currentRow < 0) {
        QMessageBox::warning(this, "提示", "请双击选择要编辑的员工");
        return;
    }

    int ekey = m_employeeTable->item(currentRow, 0)->text().toInt();
    QString ename = m_employeeTable->item(currentRow, 1)->text();

    qDebug() << "编辑员工 ID:" << ekey << "姓名:" << ename;

    DataDialog dialog(m_db, DataDialog::EmployeeDialog, DataDialog::Edit, ekey, this);

    if (dialog.exec() == QDialog::Accepted) {
        refreshEmployeeData();
        QMessageBox::information(this, "成功", "员工信息已更新");
    }
}
//
// 设置刷新按钮的统一效果
void MainMenu::setupRefreshButton(QPushButton* button, const std::function<void()>& refreshFunction)
{
    if (!button) return;

    // 保存原始样式
    QString originalStyle = button->styleSheet();
    QString originalText = button->text();

    // 连接点击事件
    connect(button, &QPushButton::clicked, this, [this, button, refreshFunction, originalStyle, originalText]() {
        // 记录原始状态
        button->setProperty("originalStyle", originalStyle);
        button->setProperty("originalText", originalText);

        // 应用闪烁效果
        animateRefreshButton(button, QColor(224, 237, 254));  // #E0EDFE

        // 执行刷新函数
        if (refreshFunction) {
            refreshFunction();
        }
    });
}

// 闪烁刷新按钮效果
void MainMenu::animateRefreshButton(QPushButton* button, const QColor& flashColor)
{
    if (!button) return;

    // 获取原始样式
    QString originalStyle = button->property("originalStyle").toString();
    QString originalText = button->property("originalText").toString();

    // 闪烁动画序列
    QTimer::singleShot(0, button, [button, flashColor]() {
        // 第一次变亮
        QString flashStyle = QString(
                                 "QPushButton {"
                                 "padding: 6px 12px;"
                                 "border-radius: 4px;"
                                 "font-size: 12px;"
                                 "min-width: 80px;"
                                 "border: 1px solid #ccc;"
                                 "background-color: %1;"  // 闪烁颜色
                                 "color: #333333;"
                                 "font-weight: bold;"
                                 "}"
                                 ).arg(flashColor.name());

        button->setStyleSheet(flashStyle);
    });

    QTimer::singleShot(100, button, [button, flashColor]() {
        // 第二次变暗一点
        QColor darkerColor = flashColor.darker(110);  // 变暗10%
        QString flashStyle2 = QString(
                                  "QPushButton {"
                                  "padding: 6px 12px;"
                                  "border-radius: 4px;"
                                  "font-size: 12px;"
                                  "min-width: 80px;"
                                  "border: 1px solid #ccc;"
                                  "background-color: %1;"  // 稍暗的颜色
                                  "color: #333333;"
                                  "font-weight: bold;"
                                  "}"
                                  ).arg(darkerColor.name());

        button->setStyleSheet(flashStyle2);
    });

    QTimer::singleShot(200, button, [button, flashColor]() {
        // 第三次变亮
        QString flashStyle3 = QString(
                                  "QPushButton {"
                                  "padding: 6px 12px;"
                                  "border-radius: 4px;"
                                  "font-size: 12px;"
                                  "min-width: 80px;"
                                  "border: 1px solid #ccc;"
                                  "background-color: %1;"  // 闪烁颜色
                                  "color: #333333;"
                                  "font-weight: bold;"
                                  "}"
                                  ).arg(flashColor.name());

        button->setStyleSheet(flashStyle3);
    });

    QTimer::singleShot(300, button, [button, originalStyle]() {
        // 恢复原始样式
        button->setStyleSheet(originalStyle);
    });
}
void MainMenu::createNoPermissionPage()
{
    qDebug() << "创建无权限页面";

    QWidget *page = new QWidget();
    page->setObjectName("noPermissionPage");
    page->setStyleSheet(
        "#noPermissionPage {"
        "background-color: #f5f7fa;"
        "}"
        );

    QVBoxLayout *layout = new QVBoxLayout(page);
    layout->setContentsMargins(0, 0, 0, 0);

    // 中间内容
    QWidget *centerWidget = new QWidget(page);
    QVBoxLayout *centerLayout = new QVBoxLayout(centerWidget);
    centerLayout->setAlignment(Qt::AlignCenter);
    centerLayout->setSpacing(15);

    // 权限图标（使用字符图标）
    QLabel *iconLabel = new QLabel("🔒", centerWidget);
    iconLabel->setStyleSheet("font-size: 80px;");
    iconLabel->setAlignment(Qt::AlignCenter);

    // 标题
    QLabel *titleLabel = new QLabel("权限受限", centerWidget);
    titleLabel->setStyleSheet(
        "QLabel {"
        "font-size: 28px;"
        "font-weight: bold;"
        "color: #34495e;"
        "}"
        );
    titleLabel->setAlignment(Qt::AlignCenter);

    // 消息
    QLabel *messageLabel = new QLabel("您的账户权限不足，无法访问此功能", centerWidget);
    messageLabel->setStyleSheet(
        "QLabel {"
        "font-size: 16px;"
        "color: #7f8c8d;"
        "}"
        );
    messageLabel->setAlignment(Qt::AlignCenter);

    // 详细信息
    QLabel *detailLabel = new QLabel(
        "当前账户: " + m_username + "\n"
                                    "职位: " + m_position + "\n"
                           "需要管理员权限才能访问员工管理模块",
        centerWidget
        );
    detailLabel->setStyleSheet(
        "QLabel {"
        "font-size: 13px;"
        "color: #95a5a6;"
        "padding: 10px;"
        "background-color: white;"
        "border-radius: 5px;"
        "border: 1px solid #e0e0e0;"
        "}"
        );
    detailLabel->setAlignment(Qt::AlignCenter);
    detailLabel->setWordWrap(true);

    // 返回按钮
    QPushButton *backButton = new QPushButton("返回主页面", centerWidget);
    backButton->setFixedSize(120, 40);
    backButton->setStyleSheet(
        "QPushButton {"
        "background-color: #3498db;"
        "color: white;"
        "border: none;"
        "border-radius: 5px;"
        "font-size: 14px;"
        "font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "background-color: #2980b9;"
        "}"
        );

    connect(backButton, &QPushButton::clicked, this, [this]() {
        m_menuList->setCurrentRow(0);
        m_contentStack->setCurrentIndex(0);
    });

    centerLayout->addStretch();
    centerLayout->addWidget(iconLabel);
    centerLayout->addWidget(titleLabel);
    centerLayout->addWidget(messageLabel);
    centerLayout->addWidget(detailLabel);
    centerLayout->addWidget(backButton);
    centerLayout->addStretch();

    layout->addWidget(centerWidget);

    m_contentStack->addWidget(page);
}

// 在 mainmenu.cpp 中添加搜索框创建函数
void MainMenu::createSearchWidget(QWidget* parent, QHBoxLayout* buttonLayout,
                                  const QString& placeholderText,
                                  const std::function<void()>& searchSlot)
{
    // 创建搜索框
    QLineEdit* searchEdit = new QLineEdit(parent);
    searchEdit->setPlaceholderText(placeholderText);
    searchEdit->setFixedWidth(200);
    searchEdit->setStyleSheet(
        "QLineEdit {"
        "padding: 5px 10px;"
        "border: 1px solid #bdc3c7;"
        "border-radius: 4px;"
        "font-size: 12px;"
        "}"
        "QLineEdit:focus {"
        "border: 1px solid #3498db;"
        "}"
        );

    // 创建搜索按钮
    QPushButton* searchButton = new QPushButton("搜索", parent);
    searchButton->setFixedSize(60, 30);
    searchButton->setStyleSheet(
        "QPushButton {"
        "background-color: #3498db;"
        "color: white;"
        "border: none;"
        "border-radius: 4px;"
        "font-size: 12px;"
        "}"
        "QPushButton:hover {"
        "background-color: #2980b9;"
        "}"
        );

    // 连接信号槽
    connect(searchButton, &QPushButton::clicked, this, [this, searchSlot]() {
        if (searchSlot) {
            searchSlot();
        }
    });

    connect(searchEdit, &QLineEdit::returnPressed, this, [this, searchSlot]() {
        if (searchSlot) {
            searchSlot();
        }
    });

    // 添加到按钮布局
    buttonLayout->addWidget(searchEdit);
    buttonLayout->addWidget(searchButton);

    // 保存搜索框引用
    if (placeholderText.contains("供应商")) {
        m_searchSupplier = searchEdit;
    } else if (placeholderText.contains("商品")) {
        m_searchProduct = searchEdit;
    } else if (placeholderText.contains("订单")) {
        m_searchOrder = searchEdit;
    } else if (placeholderText.contains("顾客")) {
        m_searchCustomer = searchEdit;
    } else if (placeholderText.contains("员工")) {
        m_searchEmployee = searchEdit;
    }
}
// mainmenu.cpp
// 搜索商品
//void MainMenu::onSearchProduct()
//{
    // if (m_searchProduct && m_productTypeFilter) {
    //     QString searchText = m_searchProduct->text().trimmed();
    //     filterTableDataWithType(m_productTable, searchText, m_productTypeFilter,
    //                             {"商品ID", "名称", "类型", "备注"});
    // }
    void MainMenu::onSearchProduct()
    {
        qDebug() << "=== 商品搜索开始 ===";

        if (!m_searchProduct) {
            qDebug() << "商品搜索框未初始化";
            return;
        }

        // 获取搜索框文本 - 这行必须添加！
        QString searchText = m_searchProduct->text().trimmed();
        qDebug() << "搜索商品文本:" << searchText;

        // 安全检查
        if (!m_db.isOpen()) {
            QMessageBox::warning(this, "错误", "数据库未连接");
            return;
        }

        // 如果搜索框为空，重新加载所有数据
        if (searchText.isEmpty()) {
            qDebug() << "搜索框为空，刷新商品数据";
            refreshProductData();
            return;
        }

        try {
            QSqlQuery query(m_db);
            query.prepare("SELECT pkey, pname, type, sprice, stock, sold, premarks "
                          "FROM product WHERE pkey LIKE :search OR pname LIKE :search "
                          "OR type LIKE :search OR premarks LIKE :search ORDER BY pkey");
            query.bindValue(":search", "%" + searchText + "%");

            if (!query.exec()) {
                QMessageBox::warning(this, "搜索错误", "查询失败：" + query.lastError().text());
                return;
            }

            m_productTable->setRowCount(0);
            int row = 0;

            while (query.next()) {
                m_productTable->insertRow(row);
                for (int col = 0; col < 7; col++) {
                    QString value = query.value(col).toString();
                    if (col == 3) {  // 售价列
                        value = QString("¥%1").arg(query.value(col).toDouble());
                    }
                    m_productTable->setItem(row, col, new QTableWidgetItem(value));
                }
                row++;
            }

            qDebug() << "商品搜索完成，找到" << row << "条记录";

        } catch (...) {
            qDebug() << "商品搜索发生异常";
            QMessageBox::critical(this, "搜索错误", "搜索时发生未知错误");
        }

        qDebug() << "=== 商品搜索结束 ===";
    }

// 搜索订单
void MainMenu::onSearchOrder()
{
    if (m_searchOrder) {
        QString searchText = m_searchOrder->text().trimmed();
        if (!searchText.isEmpty()) {
            filterTableData(m_orderTable, searchText,
                            {"订单号", "客户ID", "订单状态", "备注", "商品ID"});
        } else {
            // 搜索框为空时，显示所有数据
            refreshOrderData();
        }
    }
}
// 搜索供应商
void MainMenu::onSearchSupplier()
{
    if (m_searchSupplier) {
        QString searchText = m_searchSupplier->text().trimmed();
        filterTableData(m_supplierTable, searchText, {"供应商ID", "供应商名称"});
    }
}

// 搜索顾客
void MainMenu::onSearchCustomer()
{
    if (m_searchCustomer) {
        QString searchText = m_searchCustomer->text().trimmed();
        filterTableData(m_customerTable, searchText, {"顾客ID", "姓名", "备注"});
    }
}

// 搜索员工
void MainMenu::onSearchEmployee()
{
    if (m_searchEmployee) {
        QString searchText = m_searchEmployee->text().trimmed();
        filterTableData(m_employeeTable, searchText, {"员工号", "姓名", "职位"});
    }
}
// 通用表格过滤函数
// 通用表格过滤函数
void MainMenu::filterTableData(QTableWidget* table, const QString& searchText,
                               const QStringList& searchColumns)
{
    qDebug() << "=== 开始搜索 ===";
    qDebug() << "搜索文本:" << searchText;
    qDebug() << "表格地址:" << table;
    qDebug() << "表格行数:" << (table ? table->rowCount() : 0);
    if (!table) return;

    // 修正：toLower() 不是 toLowerCase()
    QString lowerSearchText = searchText.toLower();

    // 显示所有行
    for (int row = 0; row < table->rowCount(); row++) {
        table->setRowHidden(row, false);
    }

    // 如果没有搜索文本，显示所有行
    if (searchText.isEmpty()) {
        return;
    }

    // 搜索匹配的行
    bool foundMatch = false;
    for (int row = 0; row < table->rowCount(); row++) {
        bool match = false;

        // 搜索所有列的文本
        for (int col = 0; col < table->columnCount(); col++) {
            QTableWidgetItem* item = table->item(row, col);
            if (item) {
                QString cellText = item->text().toLower();
                if (cellText.contains(lowerSearchText)) {
                    match = true;
                    foundMatch = true;
                    break;
                }
            }
        }

        // 隐藏不匹配的行
        table->setRowHidden(row, !match);
    }

    // 如果没有找到匹配项
    if (!foundMatch) {
        QMessageBox::information(this, "搜索提示", "未找到匹配的数据");
    }
}
// 带类型筛选的表格过滤函数
void MainMenu::filterTableDataWithType(QTableWidget* table, const QString& searchText,
                                       QComboBox* typeComboBox, const QStringList& searchColumns)
{
    if (!table || !typeComboBox) return;

    QString lowerSearchText = searchText.toLower();
    QString selectedType = typeComboBox->currentText();

    // 显示所有行
    for (int row = 0; row < table->rowCount(); row++) {
        table->setRowHidden(row, false);
    }

    // 如果没有搜索文本且类型是"全部"，显示所有行
    if (searchText.isEmpty() && selectedType == "全部类型") {
        return;
    }

    bool foundMatch = false;
    for (int row = 0; row < table->rowCount(); row++) {
        bool match = true;

        // 检查类型筛选
        if (selectedType != "全部类型") {
            QTableWidgetItem* typeItem = table->item(row, 2); // 类型在第3列
            if (typeItem && typeItem->text() != selectedType) {
                match = false;
                table->setRowHidden(row, true);
                continue;
            }
        }

        // 检查搜索文本
        if (!searchText.isEmpty()) {
            bool textMatch = false;
            for (int col = 0; col < table->columnCount(); col++) {
                QTableWidgetItem* item = table->item(row, col);
                if (item) {
                    QString cellText = item->text().toLower();
                    if (cellText.contains(lowerSearchText)) {
                        textMatch = true;
                        foundMatch = true;
                        break;
                    }
                }
            }
            match = textMatch;
        } else {
            // 只有类型筛选，没有文本搜索
            foundMatch = true;
        }

        // 设置行的可见性
        table->setRowHidden(row, !match);
    }

    // 如果没有找到任何匹配项
    if (!foundMatch && (!searchText.isEmpty() || selectedType != "全部类型")) {
        for (int row = 0; row < table->rowCount(); row++) {
            table->setRowHidden(row, false);
        }
        QMessageBox::information(this, "搜索提示", "未找到匹配的数据");
    }
}

