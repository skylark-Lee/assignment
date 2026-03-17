#ifndef MAINMENU_H
#define MAINMENU_H

#include <QWidget>
#include <QPushButton>      // 添加这行
#include <QTableWidget>     // 添加这行
#include <QSplitter>        // 添加这行
#include <QListWidget>      // 添加这行
#include <QStackedWidget>   // 添加这行
#include <QSqlDatabase>

#include <QLineEdit>      // 添加这行
#include <QComboBox>

class QSplitter;
class QListWidget;
class QStackedWidget;
class QTableWidget;
class QSqlQueryModel;

class QLineEdit;         // 添加这行
class QComboBox;
class QHBoxLayout;

class MainMenu : public QWidget
{
    Q_OBJECT

public:
    explicit MainMenu(QWidget *parent = nullptr,
                      const QString& username = "",
                      const QString& position = "",
                      QSqlDatabase db = QSqlDatabase());
    ~MainMenu();

signals:
    void windowClosed();

private slots:
    void onLogoutClicked();
    void onMenuClicked(int index);

    // 刷新数据
    void refreshSupplierData();
    void refreshProductData();
    void refreshOrderData();
    void refreshCustomerData();
    void refreshEmployeeData();

    void onAddSupplierClicked();
    void onAddProductClicked();
    void onAddOrderClicked();
    void onAddCustomerClicked();
    void onAddEmployeeClicked();

    // 删除
    void onDeleteSupplierClicked();
    void onDeleteProductClicked();
    void onDeleteOrderClicked();
    void onDeleteCustomerClicked();
    void onDeleteEmployeeClicked();

    // 编辑（双击）
    void onEditSupplierClicked();
    void onEditProductClicked();
    void onEditOrderClicked();
    void onEditCustomerClicked();
    void onEditEmployeeClicked();

    //搜索
    void onSearchSupplier();
    void onSearchProduct();
    void onSearchOrder();
    void onSearchCustomer();
    void onSearchEmployee();




private:
    void setupUI();
    void setupDatabaseConnection();

    // 创建表格视图
    void createSupplierPage();
    void createProductPage();
    void createOrderPage();
    void createCustomerPage();
    void createEmployeePage();
    //
    // void createSearchWidget();
    // void filterTableData();
    // void filterTableDataWithType();
    void createSearchWidget(QWidget* parent, QHBoxLayout* buttonLayout,
                            const QString& placeholderText,
                            const std::function<void()>& searchSlot);
    void filterTableData(QTableWidget* table, const QString& searchText,
                         const QStringList& searchColumns);
    void filterTableDataWithType(QTableWidget* table, const QString& searchText,
                                 QComboBox* typeComboBox, const QStringList& searchColumns);




    // 添加数据的函数
    void addSupplier();
    void addProduct();
    void addOrder();
    void addCustomer();
    void addEmployee();

    // 删除数据的函数
    void deleteSupplier();
    void deleteProduct();
    void deleteOrder();
    void deleteCustomer();
    void deleteEmployee();
    void createNoPermissionPage();

    //按钮刷新
    void setupRefreshButton(QPushButton* button, const std::function<void()>& refreshFunction);
    void animateRefreshButton(QPushButton* button, const QColor& flashColor = QColor(224, 237, 254));


private:
    QString m_username;
    QString m_position;
    QSqlDatabase m_db;

    // 界面组件
    QSplitter *m_splitter;
    QListWidget *m_menuList;
    QStackedWidget *m_contentStack;

    // 表格和模型
    QTableWidget *m_supplierTable;
    QTableWidget *m_productTable;
    QTableWidget *m_orderTable;
    QTableWidget *m_customerTable;
    QTableWidget *m_employeeTable;

    // 按钮
    QPushButton *m_btnAddSupplier;
    QPushButton *m_btnDeleteSupplier;
    QPushButton *m_btnRefreshSupplier;

    QPushButton *m_btnAddProduct;
    QPushButton *m_btnDeleteProduct;
    QPushButton *m_btnRefreshProduct;

    QPushButton *m_btnAddOrder;
    QPushButton *m_btnDeleteOrder;
    QPushButton *m_btnRefreshOrder;

    QPushButton *m_btnAddCustomer;
    QPushButton *m_btnDeleteCustomer;
    QPushButton *m_btnRefreshCustomer;

    QPushButton *m_btnAddEmployee;
    QPushButton *m_btnDeleteEmployee;
    QPushButton *m_btnRefreshEmployee;

    //
    QLineEdit *m_searchSupplier;
    QLineEdit *m_searchProduct;
    QLineEdit *m_searchOrder;
    QLineEdit *m_searchCustomer;
    QLineEdit *m_searchEmployee;

    // 商品类型筛选
    QComboBox *m_productTypeFilter;

    QWidget *m_noPermissionPage;  // 无权限页面
};

#endif // MAINMENU_H
