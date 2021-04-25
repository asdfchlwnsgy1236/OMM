#pragma once

#include <QWidget>

#include "save.hpp"

QT_BEGIN_NAMESPACE
namespace Ui {
	class OMM;
}
QT_END_NAMESPACE

class OMM: public QWidget {
	Q_OBJECT

	public:
	OMM(QWidget *parent = nullptr);
	~OMM();

	private:
	Ui::OMM *ui;
};
