#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    this->setWindowTitle(qApp->applicationName() + " " + qApp->applicationVersion());

    CLModel = new localModel;

    menu = new QMenu(ui->historyBtn);
    populateMenusBtn();

    ui->localTree->setModel(CLModel->getTreeModel());
    ui->localList->setModel(CLModel->getListModel());
    connect(&ftp, SIGNAL(stateChanged(int)), this, SLOT(stateChangedSlot(int)));
    connect(&ftp, SIGNAL(listInfo(QUrlInfo)), this, SLOT(contentList(QUrlInfo)));
    connect(ui->serverWidget, SIGNAL(itemActivated(QTreeWidgetItem*,int)),
            this, SLOT(processItem(QTreeWidgetItem*,int)));
    connect(&ftp, SIGNAL(dataTransferProgress(qint64,qint64)),
            this, SLOT(updateDataTransferProgress(qint64,qint64)));

    ui->serverWidget->setEnabled(false);
    ui->serverWidget->setRootIsDecorated(false);
    ui->serverWidget->setHeaderLabels(QStringList() << tr("Имя") << tr("Размер") << tr("Владелец") << tr("Группа") << tr("Изменен"));
    ui->serverWidget->header()->setStretchLastSection(false);

    progressDialog = new QProgressDialog(this);
    connect(progressDialog, SIGNAL(canceled()), this, SLOT(cancelDownload()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_localTree_clicked(const QModelIndex &index)
{
    QString sPath = CLModel->getTreeModel()->fileInfo(index).absoluteFilePath();
    ui->localList->setRootIndex(CLModel->getListModel()->setRootPath(sPath));
}

void MainWindow::on_connectButton_clicked()
{
    int port = 21;
    if (ui->hostEdit->text().isEmpty()) {
        addLogString(tr("Поле для адреса хоста пусто"), error);
        return;
    }

    if (!ui->portEdit->text().isEmpty())
        port = ui->portEdit->text().toInt();

    addMenusItem(ui->hostEdit->text(), ui->userEdit->text(), ui->psswdEdit->text(), ui->portEdit->text().toInt());
    ftp.connectToHost(ui->hostEdit->text(), port);
}

void MainWindow::addLogString(QString string, LOG_STATUS status){
    if (status == error) {
        ui->textBrowser->append(QString("<p style=\"color: red; line-height:5px;\">%1</p>").arg(string));
    }

    if (status == success) {
        ui->textBrowser->append(QString("<p style=\"color: green; line-height:5px;\">%1</p>").arg(string));
    }

    if (status == neutral) {
        ui->textBrowser->append(string);
    }
}

void MainWindow::stateChangedSlot(int state) {
    QString user;
    switch(state){
        case QFtp::Connected:
            addLogString(tr("Соединение успешно"), success);
            if (ui->userEdit->text().isEmpty()) {
                user = tr("anonymous");
            } else {
                user = ui->userEdit->text();
            }
            ftp.login(user, ui->psswdEdit->text());
            addLogString(tr("Пытаемся авторизоваться на сервере..."));
            break;

        case QFtp::Unconnected:
            ///
            break;

        case QFtp::HostLookup:
            addLogString(tr("Поиск сервера..."));
            break;

        case QFtp::Connecting:
            addLogString(tr("Соединяемся с сервером..."));
            break;

        case QFtp::LoggedIn:
            addLogString(tr("Авторизация успешна"), success);
            ftp.list();
            break;

        case QFtp::Closing:
            addLogString(tr("Закрываем соединения"));
            break;
    }
}

void MainWindow::contentList(QUrlInfo info){
    addLogString(tr("Добавляем новый элемент %1").arg(info.name()), success);
    QTreeWidgetItem *item = new QTreeWidgetItem;
    item->setText(0, info.name());
    item->setText(1, QString::number(info.size()));
    item->setText(2, info.owner());
    item->setText(3, info.group());
    item->setText(4, info.lastModified().toString("MMM dd yyyy"));

    QPixmap pixmap(info.isDir() ? ":/resources/folder.png" : ":/resources/document.png");
    item->setIcon(0, pixmap);

    isDirectory[info.name()] = info.isDir();
    ui->serverWidget->addTopLevelItem(item);
    if (!ui->serverWidget->currentItem()) {
        ui->serverWidget->setCurrentItem(ui->serverWidget->topLevelItem(0));
        ui->serverWidget->setEnabled(true);
    }
}

void MainWindow::processItem(QTreeWidgetItem *item, int /*column*/){
    QString name = item->text(0);
    if (isDirectory.value(name)) {
        ui->serverWidget->clear();
        isDirectory.clear();
        currentPath += '/';
        currentPath += name;
        ftp.cd(name);
        ftp.list();
        ui->cdUp->setEnabled(true);
        addLogString(tr("Открываем %1").arg(name), success);
    }
}

void MainWindow::on_cdUp_clicked(){
    addLogString(tr("Наверх"));
    ui->serverWidget->clear();
    isDirectory.clear();
    currentPath = currentPath.left(currentPath.lastIndexOf('/'));
    if (currentPath.isEmpty()) {
        ui->cdUp->setEnabled(false);
        ftp.cd("/");
    } else {
        ftp.cd(currentPath);
    }
    ftp.list();
}

void MainWindow::on_serverWidget_itemDoubleClicked(QTreeWidgetItem *item, int column)
{
    if (isDirectory.value(item->text(0)))
        return;
    addLogString(tr("Начало загрузки файла"));
    QString fileName = CLModel->getListModel()->rootPath() + item->text(0);
    if (QFile::exists(fileName)) {
        QMessageBox::information(this, qApp->applicationName(),
                                 tr("Файл %1 уже существует "
                                    "в текущей директории.")
                                 .arg(fileName));
        return;
    }
    file = new QFile(fileName);
    if (!file->open(QIODevice::WriteOnly)) {
        QMessageBox::information(this, tr("FTP"),
                                 tr("Невозможно сохранить файл %1: %2.")
                                 .arg(fileName).arg(file->errorString()));
        delete file;
        addLogString(tr("Ошибка загрузки файла"), error);
        return;
    }

    ftp.get(ui->serverWidget->currentItem()->text(0), file);

    progressDialog->setLabelText(tr("Загрузка %1...").arg(fileName));
    progressDialog->exec();
}

void MainWindow::cancelDownload(){
    addLogString(tr("Отмена загрузки файла"));
    ftp.abort();

    if (file->exists()) {
        file->close();
    }
    delete file;
}

void MainWindow::updateDataTransferProgress(qint64 readBytes, qint64 totalBytes){
    progressDialog->setMaximum(totalBytes);
    progressDialog->setValue(readBytes);
    if (readBytes == totalBytes) {
        addLogString(tr("Передача файла окончена"), success);
        ui->serverWidget->clear();
        isDirectory.clear();
        ftp.list();
    }
}

void MainWindow::populateMenusBtn(){
    menu->clear();

    sessionsList.clear();
    QSettings settings(qApp->applicationDirPath() + "/config.ini", QSettings::IniFormat);
    QStringList groups = settings.childGroups();
    foreach (QString str, groups) {
        sessions s_sessions;
        s_sessions.host  = settings.value(tr("%1/host").arg(str), "mirror.yandex.ru").toString();
        s_sessions.port  = settings.value(tr("%1/port").arg(str), "21").toInt();
        QByteArray arr = settings.value(tr("%1/psswd").arg(str), "").toByteArray();
        s_sessions.psswd = QByteArray::fromBase64(arr);
        s_sessions.user  = settings.value(tr("%1/user").arg(str), "anonymous").toString();
        sessionsList.insert(s_sessions.host + "/" + s_sessions.user, s_sessions);
        menu->addAction(s_sessions.host + "/" + s_sessions.user,
                        this, SLOT(openMenusItem()));
    }
    ui->historyBtn->setMenu(menu);
}

void MainWindow::openMenusItem(){
    QAction *act = qobject_cast<QAction *>(sender());
    sessions s_sessions = sessionsList.value(act->text());
    ui->hostEdit->setText(s_sessions.host);
    ui->userEdit->setText(s_sessions.user);
    ui->psswdEdit->setText(s_sessions.psswd);
    if ( s_sessions.port != 0)
        ui->portEdit->setText(tr("%1").arg(s_sessions.port));
}

void MainWindow::addMenusItem(QString host, QString user, QString psswd, int port){
    QSettings settings(qApp->applicationDirPath() + "/config.ini", QSettings::IniFormat);

    QByteArray arr;
    arr.append(psswd);

    settings.setValue(tr("%1/host").arg(host), host);
    settings.setValue(tr("%1/user").arg(host), user);
    settings.setValue(tr("%1/psswd").arg(host), arr.toBase64());
    settings.setValue(tr("%1/port").arg(host), port);

    populateMenusBtn();
}

void MainWindow::on_localList_doubleClicked(const QModelIndex &index)
{
    file = new QFile(CLModel->getListModel()->fileInfo(index).filePath());
    if(file->open(QIODevice::ReadWrite)){
        QFileInfo fi(file->fileName());
        ftp.put(file, currentPath + '/' + fi.fileName());
        addLogString(tr("Загружаем файл на сервер"));
        progressDialog->setLabelText(tr("Отправка %1...").arg(fi.fileName()));
        progressDialog->exec();
        return;
    }
    addLogString(tr("Ошибка загрузки файла на сервер"), error);
}

void MainWindow::keyPressEvent(QKeyEvent * ev){
    if (ev->key() == Qt::Key_Delete) {
        QString fileName = ui->serverWidget->currentItem()->text(0);
        int ret = QMessageBox::question(this, qApp->applicationName(),
                                        tr("Вы действительно хотите удалить %1 с сервера?").arg(fileName),
                                        QMessageBox::Ok, QMessageBox::No);

        switch (ret) {
        case QMessageBox::No:
            break;
        case QMessageBox::Yes:
            ftp.remove(fileName);
            progressDialog->setLabelText(tr("Удаление %1...").arg(fileName));
            progressDialog->exec();
            break;
        }

    }
}
