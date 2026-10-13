#ifndef TEXASSOLVER_API_SERVER_PROCESS_H
#define TEXASSOLVER_API_SERVER_PROCESS_H

#include <QProcess>
#include <QString>

// Owns a TexasSolverServer child process started from the GUI's own directory.
// Must be created, started and stopped on the GUI thread.
class ServerProcess
{
    public:
        ServerProcess() = default;
        ~ServerProcess();

        ServerProcess(const ServerProcess&) = delete;
        ServerProcess& operator=(const ServerProcess&) = delete;

        // Returns false (with error filled) if the executable is missing or fails to launch.
        bool start(quint16 port, QString* error);
        void stop();
        bool isRunning() const;

        static QString executablePath();

    private:
        QProcess process_;
        quint16 port_ = 0;
};

#endif // TEXASSOLVER_API_SERVER_PROCESS_H
