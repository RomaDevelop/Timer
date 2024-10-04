#include "timer.h"

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

#include "MyQShortings.h"

Timer::Timer(QWidget *parent)
	: QWidget(parent)
{
	mainLayOut = new QVBoxLayout(this);

	auto font = this->font();
	font.setPointSize(14);
	this->setFont(font);

	checker = new QTimer(this);
	connect(checker,&QTimer::timeout,this, &Timer::SlotTick);

	auto hlo1 = new QHBoxLayout();
	QPushButton *btn_vkl = new QPushButton("Вкл", this);
	hlo1->addWidget(btn_vkl);
	connect(btn_vkl, &QPushButton::clicked, this, &Timer::SlotStartTimer);
	mainLayOut->addLayout(hlo1);

	auto hlo2 = new QHBoxLayout();
	QRadioButton *rBtnOverTime = new QRadioButton("Через");
	QRadioButton *rBtnAtTime = new QRadioButton("В заданное время");
	rBtnOverTime->setChecked(true);
	rBtnAtTime->setEnabled(false);
	hlo2->addWidget(rBtnOverTime);
	hlo2->addWidget(rBtnAtTime);
	hlo2->addStretch();
	mainLayOut->addLayout(hlo2);

	// часы
	CreateRowTime(23, hours, slider_hours);

	// минуты
	CreateRowTime(59, minutes, slider_minutes);

	// секунды
	CreateRowTime(59, seconds, slider_seconds);

	auto hlo5 = new QHBoxLayout();
	mainLayOut->addLayout(hlo5);
	QString fastButtonsStr = "1;2;3;4;5;10;15;20;30;40;60;90;120";
	auto btnNamesList = fastButtonsStr.split(";",QString::SkipEmptyParts);
	vector<QPushButton*> fastButtonsSet;
	for(auto &name:btnNamesList)
	{
		QPushButton *tmpBtn = new QPushButton(name, this);
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
	connect(btnSettings, &QPushButton::clicked, this, &Timer::SlotSettings);

	hlo8->addStretch();
	hlo8->addWidget(btnToTray);
	hlo8->addWidget(btnSettings);

	CreateTimoutWidget();
	CreateTrayIcon();
}

void Timer::CreateRowTime(int maxValue, QLineEdit *&edit, QSlider *&slider)
{
	auto hlo = new QHBoxLayout();
	QPushButton *btnPlus = new QPushButton("+", this);
	QPushButton *btnMinus = new QPushButton("-", this);
	btnPlus->setFixedSize(40,40);
	btnMinus->setFixedSize(40,40);

	slider = new QSlider(Qt::Horizontal, this);

	edit = new QLineEdit("0", this);
	edit->setFixedWidth(80);
	edit->setReadOnly(true);
	edit->setAlignment(Qt::AlignCenter);
	slider->setMaximum(maxValue);

	hlo->addWidget(btnPlus);
	hlo->addWidget(btnMinus);
	hlo->addWidget(slider);
	hlo->addWidget(edit);
	mainLayOut->addLayout(hlo);

	connect(btnPlus,&QPushButton::clicked,[slider](){ slider->setValue(slider->value()+1); });
	connect(btnMinus,&QPushButton::clicked,[slider](){ slider->setValue(slider->value()-1); });
	connect(slider,&QSlider::valueChanged,[edit](int value){ edit->setText(QSn(value)); });
}

void Timer::CreateTimoutWidget()
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

void Timer::ShowTimeoutWidget()
{
	timeOutWidget->adjustSize();
	timeOutWidget->move(this->x() + (width()-timeOutWidget->width()) / 2,
						this->y() + (height()-timeOutWidget->height()) / 2);

	QTimer::singleShot(10,[this](){ timeOutWidget->activateWindow(); });
	timeOutWidget->exec();
}

void Timer::CreateTrayIcon()
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

void Timer::Show()
{
	setWindowFlag(Qt::Tool,false);
	showMinimized();
	showNormal();
}

void Timer::SlotStartTimer()
{
	QDateTime now = QDateTime::currentDateTime();
	int hours = this->hours->text().toInt();
	int minutes = this->minutes->text().toInt();
	int seconds = this->seconds->text().toInt();
	endDateTime = now.addSecs(hours*3600 + minutes*60 + seconds);

	checker->start(20);

	editTimeForm->setText(now.time().toString("HH:mm:ss"));
	editTimeTo->setText(endDateTime.time().toString("HH:mm:ss") + " (" + QTime(hours, minutes, seconds).toString("HH:mm:ss") + ")");
}

void Timer::SlotTick()
{
	if(QDateTime::currentDateTime() >= endDateTime)
	{
		checker->stop();
		setWindowTitle("Timer");

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

	QString toolTop = windowTitle();
	QString text = editDesribtion->text();
	if(!text.isEmpty())
	{
		toolTop += " ";
		toolTop += text;
	}
	icon->setToolTip(toolTop);
}

void Timer::SlotSettings()
{
	QWidget *windowSettings = new QWidget;
	windowSettings->show();
	QGridLayout *glo = new QGridLayout(windowSettings);
	QLabel *labelSetTimers = new QLabel("Быстрые таймеры",windowSettings);
	QLineEdit *editSetTimers = new QLineEdit("1;2;3;4;5;6;7;8;10;15;30;45;60;90;120;150;180;240;",windowSettings);
	glo->addWidget(labelSetTimers,0,0);
	glo->addWidget(editSetTimers,0,1);

	QLabel *labelSetOverTimers = new QLabel("Отложить",windowSettings);
	QLineEdit *editSetOverTimers = new QLineEdit("1;2;3;4;5;6;7;8;10;15;30;45;60;",windowSettings);
	glo->addWidget(labelSetOverTimers,1,0);
	glo->addWidget(editSetOverTimers,1,1);

	QGridLayout *gloForButton = new QGridLayout;
	QPushButton *btnSaveSettings = new QPushButton("Сохранить настройки",windowSettings);
	glo->addLayout(gloForButton,2,1);
	gloForButton->addWidget(new QLabel(windowSettings),0,0);
	gloForButton->addWidget(btnSaveSettings,0,1);
	gloForButton->setColumnStretch(0,1);
	gloForButton->setColumnStretch(1,0);
}

void Timer::closeEvent(QCloseEvent *)
{
	QApplication::quit(); // потому что если окно побывало Tool приложение не закрывается
}
