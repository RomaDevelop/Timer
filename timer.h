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

class Timer : public QWidget
{
	Q_OBJECT

	QVBoxLayout *mainLayOut;

	QSystemTrayIcon *icon = nullptr;

	QLineEdit *hours;
	QLineEdit *minutes;
	QLineEdit *seconds;

	QSlider *slider_hours;
	QSlider *slider_minutes;
	QSlider *slider_seconds;

	QDateTime endDateTime;

	QLineEdit *editTimeForm;
	QLineEdit *editTimeTo;

	QLineEdit *editDesribtion;

	QTimer *checker;

	std::unique_ptr<QDialog> timeOutWidget;

	std::vector<QWidget *> widgets;
	std::vector<QWidget *> buttonsSetTimer;
	std::vector<QWidget *> buttonsContinueTimer;


public:
	Timer(QWidget *parent = nullptr);
	~Timer() = default;
	void CreateRowTime(int maxValue, QLineEdit *& edit, QSlider *&slider);
	void CreateTimoutWidget();
	void ShowTimeoutWidget();
	void CreateTrayIcon();

	void Show();

	void SlotStartTimer();
	void SlotTick();
	void SlotSettings();

	void closeEvent (QCloseEvent *event) override;
};
#endif // TIMER_H
