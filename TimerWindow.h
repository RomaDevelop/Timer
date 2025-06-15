#ifndef TIMER_H
#define TIMER_H

#include <memory>

#include <QMainWindow>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QDateTime>
#include <QTimer>
#include <QSlider>
#include <QDialog>
#include <QSystemTrayIcon>
#include <QLabel>
#include <QMediaPlayer>
#include <QCheckBox>
#include <QRadioButton>
#include <QLocalServer>
#include <QLocalSocket>

#include "AdditionalTray.h"

class TimerWindow : public QWidget
{
	Q_OBJECT

public:
	TimerWindow(QStringList args = {}, QWidget *parent = nullptr);
	~TimerWindow() = default;

	enum RowType { h, m, s };
	void CreateRowTime(RowType rowType, QLineEdit *& edit, QSlider *&slider);
	void CreateTimoutWidget();
	void CreateTrayIcon();

	void ShowMainWindow();

	void SlotControlTimer();
	void Start(const QDateTime *startTime = nullptr, const QDateTime *endTime = nullptr);
	QDateTime CalcEndTime(const QDateTime &startDateTime, const QTime &timeSelected);

	void WriteBackup();

	void SlotTick();
	void TickWhenNotActive();
	void TickWhenActive();
	QTime GetReaminTime(const QDateTime &from, const QDateTime &to);

	void PlaySound();

	void ShowOnTimeOut();

	void Finish(bool itIsTimeout, bool removeBackupFile);

	void SetWidgetsEnabled(bool timerActiveNow);

	void closeEvent (QCloseEvent *event) override;

	enum class State { active, notActive, error };
	State CurrentState();

	void RestoreBackups(const QStringList &args);
	QString backupTimersPath;
	QString backupTimerCurrent;
	QString settings_file;
	QWidget *windowSettings;
	QCheckBox *chBoxPlaySound;
	QLineEdit *leSoundFile;

	void CreateSettingsWindow();
	void LoadSettings();
	void ConnectSavingWidgets();

private:
	QVBoxLayout *mainLayOut;

	QSystemTrayIcon *icon = nullptr;

	QPushButton *btnControlTimer;

	QRadioButton *rBtnOverTime;
	QRadioButton *rBtnAtTime;

	QLineEdit *le_hours;
	QLineEdit *le_minutes;
	QLineEdit *le_seconds;

	QTime timeSelected {0,0,0};
	void UpdateFromTimeSelected();

	QSlider *slider_hours;
	QSlider *slider_minutes;
	QSlider *slider_seconds;

	QDateTime startDateTime;
	QDateTime endDateTime;

	QLineEdit *editTimeForm;
	void SetEditFrom(const QTime &time);
	QLineEdit *editTimeTo;
	void SetEditTo(const QTime &timeEnd, const QTime &timeRemain);

	QLineEdit *editDescribtion;
	bool editDescribtionIsEmpty = true;
	void SetTitleAntToolTip();

	QTimer *timerForActiveChecker;
	QMediaPlayer *player;

	std::unique_ptr<QDialog> timeOutWidget;

	std::vector<QWidget *> widgets_to_enable;

	std::shared_ptr<QLocalServer> localServer;
	std::shared_ptr<QLocalSocket> localClient;
};

#endif // TIMER_H
