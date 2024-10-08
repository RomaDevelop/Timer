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

class TimerWindow : public QWidget
{
	Q_OBJECT

	QVBoxLayout *mainLayOut;

	QSystemTrayIcon *icon = nullptr;

	QPushButton *btn_control_timer;

	QLineEdit *le_hours;
	QLineEdit *le_minutes;
	QLineEdit *le_seconds;

	QSlider *slider_hours;
	QSlider *slider_minutes;
	QSlider *slider_seconds;

	QDateTime endDateTime;

	QLineEdit *editTimeForm;
	QLineEdit *editTimeTo;

	QLineEdit *editDesribtion;

	QTimer *timer_checker;
	QMediaPlayer *player;

	std::unique_ptr<QDialog> timeOutWidget;

	std::vector<QWidget *> widgets_to_enable;

public:
	TimerWindow(QWidget *parent = nullptr);
	~TimerWindow() = default;

	void CreateRowTime(int maxValue, QLineEdit *& edit, QSlider *&slider);
	void CreateTimoutWidget();
	void ShowTimeoutWidget();
	void CreateTrayIcon();

	void Show();

	void SlotControlTimer();
	void SlotTick();

	void FinishOpts1();
	void SetWidgetsEnabled(bool value);

	void closeEvent (QCloseEvent *event) override;

	QString settings_file;
	QWidget *windowSettings;
	QCheckBox *chBoxPlaySound;
	QLineEdit *leSoundFile;

	void CreateSettingsWindow();
	void LoadSettings();
	void ConnectSavingWidgets();
};
#endif // TIMER_H
