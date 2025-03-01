#include "TimerWindow.h"

#include <QApplication>
#include <QMessageBox>

#include "MyQShortings.h"

int main(int argc, char *argv[])
{
#error сделать чтобы + и - добавляло вышестоящие или уменьшало нижние (типа 59 минут + 1 равно +час)
	QApplication a(argc, argv);
	QStringList args;
	for(int i=1; i<argc; i++) args.push_back(argv[i]);
	TimerWindow w(std::move(args));
	w.show();

	return a.exec();
}
