#include "TimerWindow.h"

#include <vector>
using namespace std;

#include <QWindow>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QSlider>
#include <QRadioButton>
#include <QDebug>
#include <QApplication>
#include <QSystemTrayIcon>
#include <QStyle>
#include <QCloseEvent>
#include <QMessageBox>
#include <QCheckBox>
#include <QFileDialog>
#include <QSettings>
#include <QComboBox>

#include "MyQShortings.h"
#include "MyQDifferent.h"

const char* on = "Включить";
const char* off = "Отключить";

namespace SettingsKeys {
	const QString do_sound_notify = "do_sound_notify";
	const QString sound_notify_file = "sound_notify_file";
}

TimerWindow::TimerWindow(QWidget *parent)
	: QWidget(parent)
{
	mainLayOut = new QVBoxLayout(this);

	auto font = this->font();
	font.setPointSize(14);
	this->setFont(font);

	timer_checker = new QTimer(this);
	connect(timer_checker,&QTimer::timeout,this, &TimerWindow::SlotTick);

	player = new QMediaPlayer(this);
	connect(player, &QMediaPlayer::mediaStatusChanged, [this](QMediaPlayer::MediaStatus status){
		if(status == QMediaPlayer::InvalidMedia)
		{
			QMessageBox::critical(this,"Error", "Can't play file " + player->media().canonicalUrl().fileName());
			player->setMedia(QUrl::fromLocalFile(""));
		}
	});

	auto hlo1 = new QHBoxLayout();
	btn_control_timer = new QPushButton(on, this);
	hlo1->addWidget(btn_control_timer);
	connect(btn_control_timer, &QPushButton::clicked, this, &TimerWindow::SlotControlTimer);
	mainLayOut->addLayout(hlo1);

	auto hlo2 = new QHBoxLayout();
	QRadioButton *rBtnOverTime = new QRadioButton("Через");
	QRadioButton *rBtnAtTime = new QRadioButton("В заданное время");
	widgets_to_enable.push_back(rBtnOverTime);
	//widgets_to_enable.push_back(rBtnAtTime);
	rBtnOverTime->setChecked(true);
	rBtnAtTime->setEnabled(false);

	chBoxPlaySound = new QCheckBox("Звуковое уведомление");

	font.setPointSize(10);
	rBtnOverTime->setFont(font);
	rBtnAtTime->setFont(font);
	chBoxPlaySound->setFont(font);

	hlo2->addWidget(rBtnOverTime);
	hlo2->addWidget(rBtnAtTime);
	hlo2->addStretch();
	hlo2->addWidget(chBoxPlaySound);

	mainLayOut->addLayout(hlo2);

	// часы
	CreateRowTime(23, le_hours, slider_hours);

	// минуты
	CreateRowTime(59, le_minutes, slider_minutes);

	// секунды
	CreateRowTime(59, le_seconds, slider_seconds);

	auto hlo5 = new QHBoxLayout();
	mainLayOut->addLayout(hlo5);
	QString fastButtonsStr = "1;2;3;4;5;10;15;20;30;40;60;90;120";
	auto btnNamesList = fastButtonsStr.split(";",QString::SkipEmptyParts);
	vector<QPushButton*> fastButtonsSet;
	for(auto &name:btnNamesList)
	{
		QPushButton *tmpBtn = new QPushButton(name, this);
		widgets_to_enable.push_back(tmpBtn);
		tmpBtn->setFixedSize(40,40);
		fastButtonsSet.push_back(tmpBtn);
		hlo5->addWidget(tmpBtn);

		connect(tmpBtn, &QPushButton::clicked, [tmpBtn, this](){
			int mCount = tmpBtn->text().toInt();
			int hCount = mCount / 60;
			mCount = mCount % 60;

			slider_hours->setValue(hCount);
			slider_minutes->setValue(mCount);
			slider_seconds->setValue(0);
		});
	}
	hlo5->addStretch();

	// от -> до
	auto hlo6 = new QHBoxLayout();
	mainLayOut->addLayout(hlo6);
	editTimeForm = new QLineEdit;
	QLabel *labelStrelka = new QLabel("->");
	editTimeTo = new QLineEdit;

	editTimeForm->setReadOnly(true);
	editTimeTo->setReadOnly(true);
	editTimeForm->setAlignment(Qt::AlignCenter);
	editTimeTo->setAlignment(Qt::AlignCenter);

	hlo6->addWidget(editTimeForm);
	hlo6->addStretch();
	hlo6->addWidget(labelStrelka);
	hlo6->addStretch();
	hlo6->addWidget(editTimeTo);

	auto hlo7 = new QHBoxLayout();
	mainLayOut->addLayout(hlo7);
	QLabel *labelDesribtion = new QLabel("Описание");
	editDesribtion = new QLineEdit;
	hlo7->addWidget(labelDesribtion);
	hlo7->addWidget(editDesribtion);

	auto hlo8 = new QHBoxLayout();
	mainLayOut->addLayout(hlo8);

	QPushButton *btnToTray = new QPushButton("Свернуть в трей", this);
	connect(btnToTray, &QPushButton::clicked, [this](){
		setWindowFlag(Qt::Tool, true);
	});
	QPushButton *btnSettings = new QPushButton("Настройки", this);
	connect(btnSettings, &QPushButton::clicked, [this](){
		windowSettings->show();
	});

	hlo8->addStretch();
	hlo8->addWidget(btnToTray);
	hlo8->addWidget(btnSettings);

	CreateTimoutWidget();
	CreateTrayIcon();

	CreateSettingsWindow();

	QTimer::singleShot(0,[this]() {
		LoadSettings();
		ConnectSavingWidgets();
	});
}

void TimerWindow::CreateRowTime(int maxValue, QLineEdit *&edit, QSlider *&slider)
{
	auto hlo = new QHBoxLayout();
	QPushButton *btnPlus = new QPushButton("+", this);
	QPushButton *btnMinus = new QPushButton("-", this);
	btnPlus->setFixedSize(40,40);
	btnMinus->setFixedSize(40,40);

	slider = new QSlider(Qt::Horizontal, this);

	edit = new QLineEdit("0", this);
	edit->setFixedWidth(60);
	edit->setReadOnly(true);
	edit->setAlignment(Qt::AlignCenter);
	slider->setMaximum(maxValue);

	hlo->addWidget(btnPlus);
	hlo->addWidget(btnMinus);
	hlo->addWidget(slider);
	hlo->addWidget(edit);
	mainLayOut->addLayout(hlo);

	widgets_to_enable.push_back(btnPlus);
	widgets_to_enable.push_back(btnMinus);
	widgets_to_enable.push_back(slider);

	connect(btnPlus,&QPushButton::clicked,[slider](){ slider->setValue(slider->value()+1); });
	connect(btnMinus,&QPushButton::clicked,[slider](){ slider->setValue(slider->value()-1); });
	connect(slider,&QSlider::valueChanged,[edit](int value){ edit->setText(QSn(value)); });
}

void TimerWindow::CreateTimoutWidget()
{
	timeOutWidget = std::make_unique<QDialog>();
	timeOutWidget->setWindowFlags(Qt::Tool);

	auto font = timeOutWidget->font();
	font.setPointSize(14);
	timeOutWidget->setFont(font);

	auto lo = new QVBoxLayout(timeOutWidget.get());
	lo->setContentsMargins(15,15,15,15);
	auto hlo1 = new QHBoxLayout;
	auto hlo2 = new QHBoxLayout;
	lo->addLayout(hlo1);
	lo->addLayout(hlo2);

	hlo1->addWidget(new QLabel("Время истекло"));

	auto btn = new QPushButton("Ok");
	hlo2->addWidget(btn);
	connect(btn,&QPushButton::clicked,[this](){
		timeOutWidget->hide();
	});
}

void TimerWindow::ShowTimeoutWidget()
{
	timeOutWidget->adjustSize();
	timeOutWidget->move(this->x() + (width()-timeOutWidget->width()) / 2,
						this->y() + (height()-timeOutWidget->height()) / 2);

	QTimer::singleShot(10,[this](){ timeOutWidget->activateWindow(); });
	timeOutWidget->exec();
}

void TimerWindow::CreateTrayIcon()
{
	if(icon)
		QMbc(this,"Error","CreateTrayIcon multiple times");

	icon = new QSystemTrayIcon(this);
	icon->setIcon(QApplication::style()->standardIcon(QStyle::SP_BrowserReload));
	icon->setToolTip("Timer");
	icon->show();
	connect(icon, &QSystemTrayIcon::activated, [this](){
		Show();
	});
}

void TimerWindow::Show()
{
	setWindowFlag(Qt::Tool,false);
	showMinimized();
	showNormal();
}

void TimerWindow::SlotControlTimer()
{
	if(btn_control_timer->text() == on)
	{
		QDateTime now = QDateTime::currentDateTime();
		int hours = this->le_hours->text().toInt();
		int minutes = this->le_minutes->text().toInt();
		int seconds = this->le_seconds->text().toInt();
		int countSecs = hours*3600 + minutes*60 + seconds;
		if(countSecs == 0) return;

		endDateTime = now.addSecs(countSecs);

		timer_checker->start(20);

		editTimeForm->setText(now.time().toString("HH:mm:ss"));
		editTimeTo->setText(endDateTime.time().toString("HH:mm:ss") + " (" + QTime(hours, minutes, seconds).toString("HH:mm:ss") + ")");
		SetWidgetsEnabled(false);
		btn_control_timer->setText(off);
	}
	else if(btn_control_timer->text() == off)
	{
		auto answ = QMessageBox::question(this, "Timer is active", "Timer is active. Are you sure you want to stop?");
		if(answ == QMessageBox::Yes) FinishOpts1();
		else if(answ == QMessageBox::No) { return; }
		else QMessageBox::critical(this, "", "unrealesed button");
	}
}

void TimerWindow::SlotTick()
{
	if(QDateTime::currentDateTime() >= endDateTime)
	{
		FinishOpts1();

		if(chBoxPlaySound->isChecked() && !leSoundFile->text().isEmpty())
		{
			player->setMedia(QUrl::fromLocalFile(leSoundFile->text()));
			player->play();
		}

		Show();
		ShowTimeoutWidget();
	}
	else
	{
		int secs = QDateTime::currentDateTime().secsTo(endDateTime);
		QTime remainTime(secs/3600, secs%3600/60, secs%60);

		editTimeTo->setText(endDateTime.time().toString("HH:mm:ss") + " (" + remainTime.toString("HH:mm:ss") + ")");

		setWindowTitle(remainTime.toString("HH:mm:ss") + " -> " +endDateTime.time().toString("HH:mm:ss")+ " Timer");
	}

	QString toolTip = windowTitle();
	QString text = editDesribtion->text();
	if(!text.isEmpty())
	{
		toolTip += " ";
		toolTip += text;
	}
	icon->setToolTip(toolTip);
}

void TimerWindow::FinishOpts1()
{
	timer_checker->stop();
	setWindowTitle("Timer");
	editTimeForm->clear();
	editTimeTo->clear();
	SetWidgetsEnabled(true);
	btn_control_timer->setText(on);
}

void TimerWindow::SetWidgetsEnabled(bool value)
{
	for(auto &w:widgets_to_enable)
		w->setEnabled(value);
}

void TimerWindow::closeEvent(QCloseEvent *event)
{
	if(timer_checker->isActive())
	{
		auto answ = QMessageBox::question(this, "Timer is active", "Timer is active. Are you sure you want to exit?");
		if(answ == QMessageBox::Yes) event->accept();
		else if(answ == QMessageBox::No) { event->ignore(); return; }
		else QMessageBox::critical(this, "", "unrealesed button");
	}

	QApplication::quit(); // потому что если окно побывало Tool приложение не закрывается
}

void TimerWindow::CreateSettingsWindow()
{
	windowSettings = new QWidget;

	QGridLayout *glo = new QGridLayout(windowSettings);
	QLabel *labelSetTimers = new QLabel("Быстрые таймеры");
	QLineEdit *editSetTimers = new QLineEdit("1;2;3;4;5;6;7;8;10;15;30;45;60;90;120;150;180;240;");
	glo->addWidget(labelSetTimers,0,0);
	glo->addWidget(editSetTimers,0,1);

	QLabel *labelSetOverTimers = new QLabel("Отложить");
	QLineEdit *editSetOverTimers = new QLineEdit("1;2;3;4;5;6;7;8;10;15;30;45;60;");
	glo->addWidget(labelSetOverTimers,1,0);
	glo->addWidget(editSetOverTimers,1,1);

	leSoundFile = new QLineEdit;
	QPushButton *btnSelSound = new QPushButton("Select");
	QObject::connect(btnSelSound, &QPushButton::clicked, [this]()
	{
		leSoundFile->setText(QFileDialog::getOpenFileName(windowSettings));
	});
	QHBoxLayout *hloSound = new QHBoxLayout;
	hloSound->addWidget(leSoundFile);
	hloSound->addWidget(btnSelSound);
	glo->addLayout(hloSound,2,0,1,-1);

	if(settings_file.isEmpty())
		settings_file = MyQDifferent::PathToExe() + "/files/settings.ini";
}

void TimerWindow::LoadSettings()
{
	QSettings settings(settings_file, QSettings::IniFormat);

	if(settings.contains(SettingsKeys::sound_notify_file))
		leSoundFile->setText(settings.value(SettingsKeys::sound_notify_file).toString());
	if(settings.contains(SettingsKeys::do_sound_notify))
		chBoxPlaySound->setChecked(settings.value(SettingsKeys::do_sound_notify).toBool());

}

void TimerWindow::ConnectSavingWidgets()
{
	connect(chBoxPlaySound, &QCheckBox::stateChanged, [this](){
		QSettings stgs(settings_file, QSettings::IniFormat);
		stgs.setValue(SettingsKeys::do_sound_notify, chBoxPlaySound->isChecked());

		if(chBoxPlaySound->isChecked() && leSoundFile->text().isEmpty())
		{
			QMessageBox::warning(this,"No sound file", "You selected sound notify, byt no sound file set");
		}
	});

	QObject::connect(leSoundFile, &QLineEdit::textChanged, [this](const QString &text){
		QSettings stgs(settings_file, QSettings::IniFormat);
		stgs.setValue(SettingsKeys::sound_notify_file, text);
	});
}
