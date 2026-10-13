#include "settingeditor.h"
#include "ui_settingeditor.h"

SettingEditor::SettingEditor(QWidget* parent) : QDialog(parent), ui(new Ui::SettingEditor) {
    ui->setupUi(this);
    this->setWindowTitle(tr("Settings"));
    QSettings setting("TexasSolver", "Setting");
    setting.beginGroup("solver");
    QString language_str = setting.value("language").toString();
    if (language_str == "EN") {
        this->ui->languageBox->setCurrentIndex(0);
    } else if (language_str == "CN") {
        this->ui->languageBox->setCurrentIndex(1);
    } else {
        qDebug().noquote() << tr("Unknown language: ") << language_str << tr("Setting fail");
    }

    int dump_round = setting.value("dump_round").toInt();
    if (dump_round > 0 && dump_round < 4) {
        this->ui->roundBox->setCurrentIndex(dump_round - 1);
    } else {
        qDebug().noquote() << tr("dump round error: ") << dump_round;
    }

    this->ui->backendBox->setCurrentIndex(setting.value("backend_mode", "local").toString() == "remote" ? 1 : 0);
    this->ui->hostEdit->setText(setting.value("server_host", "127.0.0.1").toString());
    this->ui->portBox->setValue(setting.value("server_port", 8080).toInt());

    this->initized = true;
}

SettingEditor::~SettingEditor() {
    delete ui;
}

void SettingEditor::on_confirmBox_accepted() {
    QSettings setting("TexasSolver", "Setting");
    setting.beginGroup("solver");
    QString lang = this->ui->languageBox->currentText();
    QString language_str;
    if (lang == "English") {
        language_str = "EN";
    } else if (lang == "Chinese") {
        language_str = "CN";
    } else {
        qDebug().noquote() << tr("Unknown language: ") << lang << tr("Setting fail");
    }
    setting.setValue("language", language_str);

    int round = this->ui->roundBox->currentText().toInt();
    setting.setValue("dump_round", round);

    const QString backend_mode = this->ui->backendBox->currentIndex() == 1 ? "remote" : "local";
    const QString host = this->ui->hostEdit->text().trimmed();
    const int port = this->ui->portBox->value();
    const bool backend_changed = backend_mode != setting.value("backend_mode", "local").toString() ||
                                 host != setting.value("server_host", "127.0.0.1").toString() ||
                                 port != setting.value("server_port", 8080).toInt();
    setting.setValue("backend_mode", backend_mode);
    setting.setValue("server_host", host.isEmpty() ? QString("127.0.0.1") : host);
    setting.setValue("server_port", port);

    if (backend_changed) {
        QMessageBox msgBox;
        msgBox.setText(tr("Restart program to make solver backend selection effective."));
        msgBox.exec();
    }
}

void SettingEditor::on_languageBox_currentIndexChanged(int index) {
    if (!initized)
        return;
    QString message = tr("Restart program to make language selection effective.");
    qDebug().noquote() << message;
    QMessageBox msgBox;
    msgBox.setText(message);
    msgBox.exec();
}
