#include "omm.h"

#include "ui_omm.h"

OMM::OMM(QWidget *parent): QWidget(parent), ui(new Ui::OMM) {
	ui->setupUi(this);
}

OMM::~OMM() {
	delete ui;
}
