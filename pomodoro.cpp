#include "pomodoro.h"
#include "./ui_pomodoro.h"

Pomodoro::Pomodoro(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::Pomodoro)
    , m_maxMinute { 999 }
    , m_maxSecond { 60 }
    , m_millisecondInSecond{ 1000 }
    , m_isTimeRunning { false }
    , m_isBreakRunning { false }
    , m_curMinSecTime { 0, 0 }
    , m_curMinSecBreak { 0, 0 }
    , m_timeStatus { false, false, false }
    , m_breakStatus { false, false, false }
{
    ui->setupUi(this);

    // set title
    this->setWindowTitle("Pomodoro Clock");

    // set initial values
    setLineEdit("0", ui->setTimeLineEdit, ui->timeCounter);
    setLineEdit("0", ui->setBreakLineEdit, ui->breakCounter);

    qInfo() << "constructor";
}

Pomodoro::~Pomodoro()
{
    delete ui;

    qInfo() << "destructor";
}

void Pomodoro::on_setTimeLineEdit_editingFinished()
{
    setLineEdit(ui->setTimeLineEdit->text(), ui->setTimeLineEdit, ui->timeCounter);
}

void Pomodoro::on_setBreakLineEdit_editingFinished()
{
    setLineEdit(ui->setBreakLineEdit->text(), ui->setBreakLineEdit, ui->breakCounter);
}

void Pomodoro::setLineEdit(const QString &minuteStr, QLineEdit* lineEdit, QLabel* labelCounter)
{
    // Validate minute
    if (minuteStr != "-1")
    {
        validateMinute(minuteStr, lineEdit);
    }

    // Format minute and second
    QString timeFormat = formatMinSec(lineEdit);

    // Set formatted time
    labelCounter->setText(timeFormat);
}

void Pomodoro::validateMinute(const QString minuteStr, QLineEdit* lineEdit)
{
    // Convert to int
    bool ok { false };
    int minute = minuteStr.toInt(&ok);

    // Invalid input sets to 0
    if (!ok)
    {
        minute = 0;
        lineEdit->setText(QString::number(minute));
        return;
    }

    // Check minute range
    if (minute < 0)
    {
        minute = 0;
    }
    else if (minute > m_maxMinute)
    {
        minute = m_maxMinute;
    }

    // Apply new minute
    lineEdit->setText(QString::number(minute));
}

QString Pomodoro::formatMinSec(QLineEdit* lineEditMin)
{
    // Format minute and seconds
    QString minute = lineEditMin->text();
    QString second = QString("0");
    QString hrMinFormat = QString("%1 Min %2 Sec").arg(minute, second);

    return hrMinFormat;
}

QString Pomodoro::formatMinSec(int minute, int second)
{
    QString MinSecFormat = QString("%1 Min %2 Sec").arg(QString::number(minute), QString::number(second));

    return MinSecFormat;
}


void Pomodoro::on_startTimeBtn_clicked()
{
    toggleTimer(m_timeStatus, m_curMinSecTime, ui->setTimeLineEdit, m_timeCounterTimer, ui->startTimeBtn, ui->timeCounter);
}

void Pomodoro::on_startBreakBtn_clicked()
{
    toggleTimer(m_breakStatus, m_curMinSecBreak, ui->setBreakLineEdit, m_breakCounterTimer, ui->startBreakBtn, ui->breakCounter);
}

void Pomodoro::toggleTimer(TimerStatus &timerStatus, TimeInfo &timeInfo, QLineEdit* lineEdit, QTimer*& timer, QPushButton* pushButton, QLabel* counterLabel)
{    
    // If timer is done, do nothing
    if (timerStatus.isDone)
    {
        return;
    }
    // Start new timer
    else if (!timerStatus.isRunning && !timerStatus.isPaused)
    {
        // Get initial hour and minute values
        timeInfo.minute = lineEdit->text().toInt();
        timeInfo.second = 0;

        // Connect timer slot and update clock every second
        timer = new QTimer(this);
        if (counterLabel == ui->timeCounter)
        {
            connect(timer, SIGNAL(timeout()), this, SLOT(updateClockTime()));
        }
        else
        {
            connect(timer, SIGNAL(timeout()), this, SLOT(updateClockBreak()));
        }
        timer->start(m_millisecondInSecond);

        // change button label
        pushButton->setText("Pause");

        // Set flag
        timerStatus.isRunning = true;
        timerStatus.isPaused = false;
    }
    // Pause timer
    else if (!timerStatus.isPaused)
    {
        timer->stop();
        timerStatus.isPaused = true;

        // change button label
        pushButton->setText("Resume");
    }
    else
    {
        timer->start();
        timerStatus.isPaused = false;

        // change button label
        pushButton->setText("Pause");
    }

    // Stop other time / break timer
    if (pushButton == ui->startTimeBtn && m_breakStatus.isRunning && !m_breakStatus.isPaused)
    {
        m_breakCounterTimer->stop();
        m_breakStatus.isPaused = true;

        ui->startBreakBtn->setText("Resume");
    }
    else if (pushButton == ui->startBreakBtn && m_timeStatus.isRunning && !m_timeStatus.isPaused)
    {
        m_timeCounterTimer->stop();
        m_timeStatus.isPaused = true;

        ui->startTimeBtn->setText("Resume");
    }
}

void Pomodoro::updateClockTime()
{
    // Check and decrement time
    // Lapse second
    if (m_curMinSecTime.second >= 1)
    {
        m_curMinSecTime.second -= 1;

        // Format hour and minute
        QString timeFormat = formatMinSec(m_curMinSecTime.minute, m_curMinSecTime.second);

        // Set formatted time
        ui->timeCounter->setText(timeFormat);
    }
    // Lapse minute
    else if (m_curMinSecTime.second == 0 && m_curMinSecTime.minute >= 1)
    {
        m_curMinSecTime.minute -= 1;
        m_curMinSecTime.second = m_maxSecond - 1;

        // Format hour and minute
        QString timeFormat = formatMinSec(m_curMinSecTime.minute, m_curMinSecTime.second);

        // Set formatted time
        ui->timeCounter->setText(timeFormat);
    }
    // Disconnect timer slot
    else
    {
        ui->timeCounter->setText("Done!");
        disconnect(m_timeCounterTimer, SIGNAL(timeout()), this, SLOT(updateClockTime()));

        // change button label
        ui->startTimeBtn->setText("---");

        // Unset flag
        m_timeStatus.isRunning = false;
        m_timeStatus.isPaused = false;
        m_timeStatus.isDone = true;
    }
}

void Pomodoro::updateClockBreak()
{
    // Check and decrement time
    // Lapse second
    if (m_curMinSecBreak.second >= 1)
    {
        m_curMinSecBreak.second -= 1;

        // Format hour and minute
        QString timeFormat = formatMinSec(m_curMinSecBreak.minute, m_curMinSecBreak.second);

        // Set formatted time
        ui->breakCounter->setText(timeFormat);
    }
    // Lapse minute
    else if (m_curMinSecBreak.second == 0 && m_curMinSecBreak.minute >= 1)
    {
        m_curMinSecBreak.minute -= 1;
        m_curMinSecBreak.second = m_maxSecond - 1;

        // Format hour and minute
        QString timeFormat = formatMinSec(m_curMinSecBreak.minute, m_curMinSecBreak.second);

        // Set formatted time
        ui->breakCounter->setText(timeFormat);
    }
    // Disconnect timer slot
    else
    {
        ui->breakCounter->setText("Done!");
        disconnect(m_breakCounterTimer, SIGNAL(timeout()), this, SLOT(updateClockBreak()));

        // change button label
        ui->startBreakBtn->setText("---");

        // Unset flag
        m_breakStatus.isRunning = false;
        m_breakStatus.isPaused = false;
        m_breakStatus.isDone = true;
    }
}

void Pomodoro::on_resetTimeBtn_clicked()
{
    disconnect(m_timeCounterTimer, SIGNAL(timeout()), this, SLOT(updateClockTime()));

    // set initial values
    setLineEdit("-1", ui->setTimeLineEdit, ui->timeCounter);

    // change button label
    ui->startTimeBtn->setText("Start");

    // Unset flag
    m_timeStatus.isRunning = false;
    m_timeStatus.isPaused = false;
    m_timeStatus.isDone = false;
}


void Pomodoro::on_resetBreakBtn_clicked()
{
    disconnect(m_breakCounterTimer, SIGNAL(timeout()), this, SLOT(updateClockBreak()));

    // set initial values
    setLineEdit("-1", ui->setBreakLineEdit, ui->breakCounter);

    // change button label
    ui->startBreakBtn->setText("Start");

    // Unset flag
    m_breakStatus.isRunning = false;
    m_breakStatus.isPaused = false;
    m_breakStatus.isDone = false;
}

