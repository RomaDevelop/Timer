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

class TimerWindow : public QWidget
{
	Q_OBJECT

	QVBoxLayout *mainLayOut;

	QSystemTrayIcon *icon = nullptr;

	QPushButton *btn_control_timer;

	QRadioButton *rBtnOverTime;
	QRadioButton *rBtnAtTime;

	QLineEdit *le_hours;
	QLineEdit *le_minutes;
	QLineEdit *le_seconds;

	QSlider *slider_hours;
	QSlider *slider_minutes;
	QSlider *slider_seconds;

	QDateTime startDateTime;
	QDateTime endDateTime;

	QLineEdit *editTimeForm;
	QLineEdit *editTimeTo;

	QLineEdit *editDesribtion;

	QTimer *timer_checker;
	QMediaPlayer *player;

	std::unique_ptr<QDialog> timeOutWidget;

	std::vector<QWidget *> widgets_to_enable;

public:
	TimerWindow(QStringList args = {}, QWidget *parent = nullptr);
	~TimerWindow() = default;

	void CreateRowTime(int maxValue, QLineEdit *& edit, QSlider *&slider);
	void CreateTimoutWidget();
	void CreateTrayIcon();

	void ShowMainWindow();

	void SlotControlTimer();
	void Start(const QDateTime *startTime = nullptr, const QDateTime *endTime = nullptr);

	void WriteBackup();

	void SlotTick();
	QTime GetReaminTime();

	void PlaySound();

	void ShowOnTimeOut();

	void Finish(bool itIsTimeout, bool removeBackupFile);

	void SetWidgetsEnabled(bool value);

	void closeEvent (QCloseEvent *event) override;

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
};
#endif // TIMER_H
