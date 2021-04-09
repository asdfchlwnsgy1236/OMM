#include <QApplication>

#include "omm.h"

int main(int argc, char *argv[]) {
	QApplication a(argc, argv);
	OMM w;
	w.show();
	return a.exec();
}
