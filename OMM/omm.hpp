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

	private:
	Ui::OMM *ui;
	// The current save file.
	omm::Save save;

	public:
	OMM(QWidget *parent = nullptr);
	~OMM();
};
