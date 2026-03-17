#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSqlDatabase>
#include <QSqlQuery>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainMenu;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_loginbutton_clicked();
    void returnToLogin();
    //
    //void setupSimpleLoginUI();



private:
    Ui::MainWindow *ui;

    QSqlDatabase m_db;
    MainMenu *m_mainMenu;
    bool connectToDatabase();
    bool verifyLogin(const QString& username, const QString& password,
                     QString& employeeName, QString& position);

};
#endif // MAINWINDOW_H
