#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "mainmenu.h"

#include <QCoreApplication>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QDebug>
#include <QTimer>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QListWidget>
#include <QSplitter>
#include <QStackedWidget>
#include <QTableWidget>
#include <QHeaderView>

#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QInputDialog>
#include <QLineEdit>
#include <QMessageBox>
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_db(QSqlDatabase::addDatabase("QODBC"))
    , m_mainMenu(nullptr)
{
    ui->setupUi(this);

    //
     //setBackgroundImage(":/images/jpg.jpg");


    QString connectionString =
        "DRIVER={ODBC Driver 17 for SQL Server};"
        "SERVER=LAPTOP-23BEH08C;"
        "DATABASE=mgsystem;"
        "Trusted_Connection=yes;";
    m_db.setDatabaseName(connectionString);
    m_db.setDatabaseName(connectionString);
    // 设置连接
    // 3. 尝试打开连接并检查错误
    if (m_db.open()) {
        qDebug() << "数据库连接成功！";
        // 连接成功，可以执行查询了
    } else {
        qDebug() << "连接失败，错误信息：" << m_db.lastError().text();
        // 这里会输出更详细的错误，帮助进一步诊断
    }
    setWindowTitle("商贸管理系统");
    ui->password->setEchoMode(QLineEdit::Password);
    connect(ui->loginbutton, &QPushButton::clicked,
            this, &MainWindow::on_loginbutton_clicked);
}

MainWindow::~MainWindow()
{
    if (m_db.isOpen()) {
        m_db.close();
    }
    if (m_mainMenu) {
        m_mainMenu->deleteLater();
    }
    delete ui;
}

bool MainWindow::verifyLogin(const QString& username, const QString& password,
                             QString& employeeName, QString& position)
{
    if (!m_db.isOpen()) {
        QMessageBox::warning(this, "错误", "数据库未连接");
        return false;
    }

    QSqlQuery query;

    //直接查询（如果username是ekey的话）
    bool ok;
    int ekey = username.toInt(&ok);

    if (ok) {
        // 使用ekey作为主键查询
        query.prepare("SELECT ename, position FROM employee WHERE ekey = :ekey AND password = :password");
        query.bindValue(":ekey", ekey);
    } else {
        // 使用用户名查询（如果需要的话）
        query.prepare("SELECT ename, position FROM employee WHERE ename = :name AND password = :password");
        query.bindValue(":name", username);
    }

    query.bindValue(":password", password);

    if (!query.exec()) {
        QMessageBox::warning(this, "查询错误",
                             "查询失败: " + query.lastError().text());
        return false;
    }

    if (query.next()) {
        employeeName = query.value(0).toString();
        position = query.value(1).toString();
        return true;
    }

    return false;
}




void MainWindow::on_loginbutton_clicked()
{
    //
    static bool isCreating = false;
    if (isCreating) {
        return;
    }

    isCreating = true;

    QString username = ui->ekey->text().trimmed();
    QString password = ui->password->text();

    // 输入验证
    if (username.isEmpty()) {
        QMessageBox::warning(this, "输入错误", "请输入账号");
        ui->ekey->setFocus();
        return;
    }

    if (password.isEmpty()) {
        QMessageBox::warning(this, "输入错误", "请输入密码");
        ui->password->setFocus();
        return;
    }
    QString employeeName, position;
    if (verifyLogin(username, password, employeeName, position)) {
        // 登录成功
        this->hide();

        // 传递数据库连接给主菜单
        MainMenu *mainMenu = new MainMenu(nullptr, employeeName, position, m_db);

        connect(mainMenu, &MainMenu::windowClosed, this, [this]() {
            this->show();
            ui->ekey->clear();
            ui->password->clear();
            this->hide();
        });

        mainMenu->show();

    } else {
        QMessageBox::warning(this, "登录失败", "账号或密码错误");
        ui->ekey->clear();
        ui->ekey->setFocus();
        ui->password->clear();
        ui->password->setFocus();
    }

}
void MainWindow::returnToLogin()
{
    // 显示登录窗口
    this->show();

    // 销毁主菜单窗口
    if (m_mainMenu) {
        m_mainMenu->deleteLater();
        m_mainMenu = nullptr;
    }
}
