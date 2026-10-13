#include "include/api/server_process.h"
#include "include/api/protocol.h"

#include <QCoreApplication>
#include <QFileInfo>
#include <QHostAddress>
#include <QTcpSocket>

#ifdef Q_OS_WIN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

ServerProcess::~ServerProcess()
{
    stop();
}

QString ServerProcess::executablePath()
{
#ifdef Q_OS_WIN
    const QString name = "TexasSolverServer.exe";
#else
    const QString name = "TexasSolverServer";
#endif
    return QCoreApplication::applicationDirPath() + "/" + name;
}

bool ServerProcess::start(quint16 port, QString* error)
{
    const QString program = executablePath();
    if (!QFileInfo::exists(program)) {
        if (error != nullptr)
            *error = QString("solver server not found: %1").arg(program);
        return false;
    }

    port_ = port;
    process_.setProgram(program);
    process_.setArguments({QString::number(port)});
    process_.setWorkingDirectory(QCoreApplication::applicationDirPath());
    // The server logs every iteration to stdout; nobody reads it here, and an unread pipe would block it.
    process_.setStandardOutputFile(QProcess::nullDevice());
    process_.setProcessChannelMode(QProcess::ForwardedErrorChannel);
#ifdef Q_OS_WIN
    process_.setCreateProcessArgumentsModifier(
        [](QProcess::CreateProcessArguments* args) { args->flags |= CREATE_NO_WINDOW; });
#endif

    process_.start();
    if (!process_.waitForStarted(5000)) {
        if (error != nullptr)
            *error = QString("failed to start solver server: %1").arg(process_.errorString());
        return false;
    }
    return true;
}

bool ServerProcess::isRunning() const
{
    return process_.state() != QProcess::NotRunning;
}

void ServerProcess::stop()
{
    if (!isRunning())
        return;

    QTcpSocket socket;
    socket.connectToHost(QHostAddress::LocalHost, port_);
    if (socket.waitForConnected(1000)) {
        socket.write(encodeMessage({{"cmd", "shutdown"}}));
        socket.waitForBytesWritten(1000);
        socket.waitForReadyRead(1000);
    }

    if (!process_.waitForFinished(3000)) {
        process_.kill();
        process_.waitForFinished(2000);
    }
}
