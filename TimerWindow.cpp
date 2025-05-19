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

#include "CodeMarkers.h"
#include "PlatformDependent.h"
#include "MyQShortings.h"
#include "MyQDifferent.h"
#include "MyCppDifferent.h"
#include "MyQDialogs.h"
#include "MyQExecute.h"

namespace ButtonCaptions {
	const char* on = "Включить";
	const char* off = "Отключить";
}

namespace SettingsKeys {
	const QString do_sound_notify = "do_sound_notify";
	const QString sound_notify_file = "sound_notify_file";
}

TimerWindow::TimerWindow(QStringList args, QWidget *parent)
	: QWidget(parent)
{
	qdbg << "предварительная загрузка медиаплеера";
	qdbg << "нужно сделать регулятор громкости";
	qdbg << "LAV audio decoder (или другие кодеки) выгружать просле завершения работы плеера";
	qdbg << "добавить ресурсы в класс ресурсы";
	
	mainLayOut = new QVBoxLayout(this);

	auto font = this->font();
	font.setPointSize(14);
	this->setFont(font);

	timerForActiveChecker = new QTimer(this);
	connect(timerForActiveChecker,&QTimer::timeout,this, &TimerWindow::SlotTick);
	timerForActiveChecker->start(20);

	player = new QMediaPlayer(this);
	connect(player, &QMediaPlayer::mediaStatusChanged, [this](QMediaPlayer::MediaStatus status){
		if(status == QMediaPlayer::InvalidMedia)
		{
			QMessageBox::critical(this,"Error", "Can't play file " + player->media().canonicalUrl().fileName());
			player->setMedia(QUrl::fromLocalFile(""));
		}
	});


	// включить / отключить
	auto hlo1 = new QHBoxLayout();
	btnControlTimer = new QPushButton(ButtonCaptions::on, this);
	hlo1->addWidget(btnControlTimer);
	connect(btnControlTimer, &QPushButton::clicked, this, &TimerWindow::SlotControlTimer);
	mainLayOut->addLayout(hlo1);

	auto hlo2 = new QHBoxLayout();
	rBtnOverTime = new QRadioButton("Через");
	rBtnAtTime = new QRadioButton("В заданное время");
	widgets_to_enable.push_back(rBtnOverTime);
	widgets_to_enable.push_back(rBtnAtTime);
	rBtnOverTime->setChecked(true);

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
	CreateRowTime(h, le_hours, slider_hours);

	// минуты
	CreateRowTime(m, le_minutes, slider_minutes);

	// секунды
	CreateRowTime(s, le_seconds, slider_seconds);

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
	MyQWidget::SetTextColor_palette(editTimeForm, Qt::gray);
	MyQWidget::SetTextColor_palette(editTimeTo, Qt::gray);
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
	connect(editDesribtion, &QLineEdit::textChanged, [this](const QString &){
		if(0) CodeMarkers::to_do("сделать механизм отложенной записи, причем чтобы записывало не на каждую букву,"
								 "а чтобы записывал уже по окончанию редактирования");
		if(CurrentState() == State::active)
			WriteBackup();
	});

	auto hlo8 = new QHBoxLayout();
	mainLayOut->addLayout(hlo8);

	QPushButton *btnToTray = new QPushButton("Свернуть в трей", this);
	connect(btnToTray, &QPushButton::clicked, [this](){
		if(CurrentState() != State::active)
		{
			auto res = QMessageBox::question(nullptr, "Minimize", "Timer is not started. Confirm minimize?");
			if(res == QMessageBox::No) return;
		}

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

	backupTimersPath = MyQDifferent::PathToExe() + "/files/backup_timers";
	if(!QDir().mkpath(backupTimersPath)) QMbc(0,"err","can't create path " + backupTimersPath);

	QTimer::singleShot(0,[this, args]() {
		RestoreBackups(args);
	});

	QTimer::singleShot(0,[this]() {
		LoadSettings();
		ConnectSavingWidgets();
	});
}

void TimerWindow::CreateRowTime(RowType rowType, QLineEdit *&edit, QSlider *&slider)
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
	slider->setMaximum(rowType == h ? 23 : 59);

	hlo->addWidget(btnPlus);
	hlo->addWidget(btnMinus);
	hlo->addWidget(slider);
	hlo->addWidget(edit);
	mainLayOut->addLayout(hlo);

	widgets_to_enable.push_back(btnPlus);
	widgets_to_enable.push_back(btnMinus);
	widgets_to_enable.push_back(slider);

	connect(btnPlus,&QPushButton::clicked,[this, rowType](){
		if(rowType == h) { timeSelected = timeSelected.addSecs(60*60); }
		else if(rowType == m) { timeSelected = timeSelected.addSecs(60); }
		else if(rowType == s) { timeSelected = timeSelected.addSecs(1); }
		else QMbError("wrong rowType " + QSn(rowType));

		UpdateFromTimeSelected();
	});
	connect(btnMinus,&QPushButton::clicked,[this, rowType](){
		if(timeSelected == QTime(0,0,0)) return;

		if(rowType == h) { timeSelected = timeSelected.addSecs(-60*60); }
		else if(rowType == m) { timeSelected = timeSelected.addSecs(-60); }
		else if(rowType == s) { timeSelected = timeSelected.addSecs(-1); }
		else QMbError("wrong rowType " + QSn(rowType));

		UpdateFromTimeSelected();
	});

	connect(slider,&QSlider::valueChanged,[this, rowType](int value){

		if(rowType == h) timeSelected.setHMS(value, timeSelected.minute(), timeSelected.second());
		else if(rowType == m) timeSelected.setHMS(timeSelected.hour(), value, timeSelected.second());
		else if(rowType == s) timeSelected.setHMS(timeSelected.hour(), timeSelected.minute(), value);
		else QMbError("wrong rowType " + QSn(rowType));

		UpdateFromTimeSelected();
	});
}

void TimerWindow::CreateTimoutWidget()
{
	timeOutWidget = std::make_unique<QDialog>(this, Qt::Tool);
	timeOutWidget->setWindowFlags(windowFlags()
								  & ~Qt::WindowCloseButtonHint
								  & ~Qt::WindowMinimizeButtonHint
								  & ~Qt::WindowMaximizeButtonHint);

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
		player->stop();
		PlatformDependent::SetTopMost(this, false);
	});

	timeOutWidget->adjustSize();
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
		ShowMainWindow();
	});
}

void TimerWindow::ShowMainWindow()
{
	setWindowFlag(Qt::Tool,false);
	showNormal();
	PlatformDependent::SetTopMost(this, true);
	PlatformDependent::SetTopMost(this, false);
}

void TimerWindow::SlotControlTimer()
{
	if(CurrentState() == State::notActive)
	{
		Start();
	}
	else if(CurrentState() == State::active)
	{
		auto answ = QMessageBox::question(this, "Timer is active", "Timer is active. Are you sure you want to stop?");
		if(answ == QMessageBox::Yes) Finish(false, true);
		else if(answ == QMessageBox::No) { return; }
		else QMessageBox::critical(this, "", "unrealesed button");
	}
}

void TimerWindow::Start(const QDateTime *startTime, const QDateTime *endTime)
{
	if(startTime && startTime->isValid() && endTime && endTime->isValid()) {}
	else if(startTime || endTime) QMbError("TimerWindow::Start wrong startTime/endTime");

	startDateTime = QDateTime::currentDateTime();
	if(startTime && startTime->isValid()) startDateTime = *startTime;

	if(endTime && endTime->isValid())
	{
		int countSecs = startTime->secsTo(*endTime);
		int hours = countSecs / 3600; // 1 час = 3600 секунд
		int minutes = (countSecs % 3600) / 60; // Остаток секунд делим на 60 для получения минут
		int seconds = countSecs % 60; // Остаток секунд
		timeSelected = QTime(hours, minutes, seconds);
		UpdateFromTimeSelected();
	}

	endDateTime = CalcEndTime(startDateTime, timeSelected);

	SetEditFrom(startDateTime.time());
	SetWidgetsEnabled(true);
	btnControlTimer->setText(ButtonCaptions::off);

	backupTimerCurrent = backupTimersPath + "/" + startDateTime.toString("yyyy.MM.dd hh-mm-ss-zzz") + ".txt";
	WriteBackup();
}

QDateTime TimerWindow::CalcEndTime(const QDateTime &startDateTime, const QTime &timeSelected)
{
	if(rBtnOverTime->isChecked())
		return startDateTime.addSecs(QTime(0,0,0).secsTo(timeSelected));
	else if(rBtnAtTime->isChecked())
	{
		auto endDateTime = QDateTime::currentDateTime();
		endDateTime.setTime(QTime(timeSelected.hour(), timeSelected.minute(), timeSelected.second()));
		if(endDateTime < startDateTime) endDateTime = endDateTime.addDays(1);
		return endDateTime;
	}
	QMbError("");
	return QDateTime{};
}

void TimerWindow::WriteBackup()
{
	QString contentToWrite;
	contentToWrite += QSn(QCoreApplication::applicationPid());
	contentToWrite += "\n";
	contentToWrite += PlatformDependent::GetProcessStartTime(
				QCoreApplication::applicationPid()).toString("yyyy.MM.dd hh-mm-ss-zzz");
	contentToWrite += "\n";
	contentToWrite += startDateTime.toString("yyyy.MM.dd hh-mm-ss-zzz");
	contentToWrite += "\n";
	contentToWrite += endDateTime.toString("yyyy.MM.dd hh-mm-ss-zzz");
	contentToWrite += "\n";
	contentToWrite += editDesribtion->text();
	if(!MyQFileDir::WriteFile(backupTimerCurrent, contentToWrite))
		QMbc(0,"error","error write file [" + backupTimerCurrent + "]");
}

void TimerWindow::SlotTick()
{
	if(CurrentState() == State::notActive)
	{
		TickWhenNotActive();
	}
	else if(CurrentState() == State::active)
	{
		TickWhenActive();
	}
}

void TimerWindow::TickWhenNotActive()
{
	auto curDT = QDateTime::currentDateTime();
	SetEditFrom(curDT.time());
	auto endDT = CalcEndTime(curDT, timeSelected);
	SetEditTo(endDT.time(), GetReaminTime(curDT, endDT));
}

void TimerWindow::TickWhenActive()
{
	if(QDateTime::currentDateTime() >= endDateTime)
	{
		Finish(true, true);
	}
	else
	{
		auto remainTime = GetReaminTime(QDateTime::currentDateTime(), endDateTime);
		SetEditTo(endDateTime.time(), remainTime);

		QString title = remainTime.toString("HH:mm:ss");
		title += " ";
		title += editDesribtion->text();
		setWindowTitle(title);
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

QTime TimerWindow::GetReaminTime(const QDateTime &from, const QDateTime &to)
{
	int secs = from.secsTo(to);
	QTime remainTime(secs/3600, secs%3600/60, secs%60);
	return remainTime;
}

void TimerWindow::PlaySound()
{
	auto soundFile = leSoundFile->text();
	if(chBoxPlaySound->isChecked() && !soundFile.isEmpty())
	{
		if(!QFileInfo(soundFile).isFile()) { QMbc(0,"Error","Sound file "+soundFile+" not exists!"); return; }
		player->setMedia(QUrl::fromLocalFile(soundFile));
		player->play();
	}
}

void TimerWindow::ShowOnTimeOut()
{
	ShowMainWindow();
	PlatformDependent::SetTopMost(this, true);
	QTimer::singleShot(0,[this](){ timeOutWidget->exec(); });
}

void TimerWindow::Finish(bool itIsTimeout, bool removeBackupFile)
{
	setWindowTitle("Timer");
	SetWidgetsEnabled(false);
	btnControlTimer->setText(ButtonCaptions::on);

	if(removeBackupFile)
		if(!QFile::remove(backupTimerCurrent)) QMbc(0,"error","error removing file [" + backupTimerCurrent + "]");

	if(itIsTimeout)
	{
		PlaySound();
		ShowOnTimeOut();
	}
}

void TimerWindow::SetWidgetsEnabled(bool timerActiveNow)
{
	if(timerActiveNow)  // если активен - дизэйблим и черный цвет
	{
		for(auto &w:widgets_to_enable) w->setEnabled(true);
		MyQWidget::SetTextColor_palette(editTimeForm, Qt::black);
		MyQWidget::SetTextColor_palette(editTimeTo, Qt::black);
	}
	else	// иначе - энейблим и сервый цвет
	{
		for(auto &w:widgets_to_enable) w->setEnabled(false);
		MyQWidget::SetTextColor_palette(editTimeForm, Qt::gray);
		MyQWidget::SetTextColor_palette(editTimeTo, Qt::gray);
	}
}

void TimerWindow::closeEvent(QCloseEvent *event)
{
	if(CurrentState() == State::active)
	{
		auto answ = MyQDialogs::CustomDialog("Timer is active", "You are closing active timer. Choose action:"
											 "\n\n(choose Close and save backup and you can restore current timer at next launch)",
											 {"Close", "Close and save backup", "Abort close"});
		if(answ == "Close") { Finish(false, true); event->accept(); }
		else if(answ == "Close and save backup") { Finish(false, false); event->accept(); }
		else if(answ == "Abort close") { event->ignore(); return; }
		else QMessageBox::critical(this, "", "unrealesed button");
	}

	QApplication::quit(); // потому что если окно побывало Tool приложение не закрывается
}

TimerWindow::State TimerWindow::CurrentState()
{
	QString btnText = btnControlTimer->text();
	if(btnText == ButtonCaptions::on)
		return State::notActive;
	else if(btnText == ButtonCaptions::off)
		return State::active;
	QMbError("TimerWindow::CurrentState cant define state, wrong btn text: " + btnText);
	return State::error;
}

void TimerWindow::RestoreBackups(const QStringList &args)
{
	if(!args.isEmpty())
	{
		auto contentsList = MyQFileDir::ReadFile1(backupTimersPath + "/" + args[0]).split("\n");

		if(!QFile(backupTimersPath + "/" + args[0]).remove()) QMbError("can't remove");

		if(contentsList.size() != 5) QMbError("wrong size " + contentsList.join("\n"));
		else
		{
			editDesribtion->setText(contentsList[4]);
			auto start = QDateTime::fromString(contentsList[2], "yyyy.MM.dd hh-mm-ss-zzz");
			auto end = QDateTime::fromString(contentsList[3], "yyyy.MM.dd hh-mm-ss-zzz");
			Start(&start, &end);
			QPoint pos(args[1].toInt(), args[2].toInt());
			QTimer::singleShot(0,[this, pos](){ move(pos); });
		}
		return;
	}

	QStringList PIDs, pStartedAts, starts, finishes, captions;
	QFileInfoList correctFiles;
	auto filesInfos = QDir(backupTimersPath).entryInfoList(QDir::Files);
	for(auto &fileInfo:filesInfos)
	{
		auto readRes = MyQFileDir::ReadFile2(fileInfo.filePath());
		auto rowsInContent = readRes.content.split("\n");

		if(fileInfo.isFile() && fileInfo.suffix() == "txt" && readRes.success && rowsInContent.size() == 5)
		{
			auto pid = rowsInContent[0];
			auto startedAt = rowsInContent[1];

			if(PlatformDependent::IsProcessRunning(pid.toUInt()) &&
					QDateTime::fromString(startedAt,"yyyy.MM.dd hh-mm-ss-zzz") == PlatformDependent::GetProcessStartTime(pid.toUInt()))
				continue;

			starts.push_back(rowsInContent[2]);
			PIDs.push_back(pid);
			pStartedAts.push_back(startedAt);
			finishes.push_back(rowsInContent[3]);
			captions.push_back(rowsInContent[4]);
			correctFiles.push_back(fileInfo);
		}
		else
		{
			auto answ = MyQDialogs::CustomDialog("Error", "Wrong backup file " + fileInfo.filePath(),
												 {"Show text and remove", "Ignore and remove"});
			if(0) {}
			else if(answ == "Show text and remove")
			{
				auto rRes = MyQFileDir::ReadFile2(fileInfo.filePath());
				if(rRes.success)
				{
					if(!QFile(fileInfo.filePath()).remove()) QMbError("can't remove");
					MyQDialogs::ShowText(rRes.content);
				}
				else QMbError("can't read, will not remove");
			}
			else if(answ == "Ignore and remove") { if(!QFile(fileInfo.filePath()).remove()) QMbError("can't remove"); }
			else QMbError("unexpacted answ " + answ);
		}
	}


	bool removeAll = false;
	bool restoreAll = false;
	QPoint pos = this->pos();
	for(int i=0; i<starts.size(); i++)
	{
		QString text = "Timer backup found:\n\nstarted at: " + starts[i] + "\nfinished at: " + finishes[i];
		if(!captions[i].isEmpty()) text += "\ndescribtion: " + captions[i];
		else text += "\ndescribtion empty";
		QString answ;
		if(removeAll) answ = "Remove";
		else if(restoreAll) answ = "Restore";

		else answ = MyQDialogs::CustomDialog("Timer backup", text,
											 {"Restore", "Restore all", "Ignore", "Ignore all", "Remove", "Remove all"});
		if(0) {}
		else if(answ == "Restore") {
			pos.setX(pos.x()+30);
			pos.setY(pos.y()+30);
			MyQExecute::Execute(MyQDifferent::ExeNameWithPath(), {correctFiles[i].fileName(), QSn(pos.x()), QSn(pos.y())});
			// не удалять файл!!! его удалит запущенная прога!!!
			MyCppDifferent::sleep_ms(100);
		}
		else if(answ == "Restore all") { restoreAll = true; i--; /*i-- чтобы остаться на этом же файле*/ }
		else if(answ == "Ignore") { continue; }
		else if(answ == "Ignore all") { break; }
		else if(answ == "Remove") { if(!QFile(filesInfos[i].filePath()).remove()) QMbError("can't remove"); }
		else if(answ == "Remove all") { removeAll = true; i--; /*i-- чтобы остаться на этом же файле*/ }
		else QMbError("unexpacted answ " + answ);
	}

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

void TimerWindow::UpdateFromTimeSelected() {
	le_hours->setText(QSn(timeSelected.hour()));
	le_minutes->setText(QSn(timeSelected.minute()));
	le_seconds->setText(QSn(timeSelected.second()));

	slider_hours->setValue(timeSelected.hour());
	slider_minutes->setValue(timeSelected.minute());
	slider_seconds->setValue(timeSelected.second());
}

void TimerWindow::SetEditFrom(const QTime &time) {
	editTimeForm->setText(time.toString("HH:mm:ss"));
}

void TimerWindow::SetEditTo(const QTime &timeEnd, const QTime &timeRemain) {
	editTimeTo->setText(timeEnd.toString("HH:mm:ss") + " (" + timeRemain.toString("HH:mm:ss") + ")");
}
