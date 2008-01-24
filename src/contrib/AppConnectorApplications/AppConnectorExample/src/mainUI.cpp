/***************************************************************************
 *   Copyright (C) 2007 by J. Adam Wilson   *
 *   jawilson@cae.wisc.edu   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include <QtGui>
#include "mainUI.h"

mainUI::mainUI()
{
	// setupUI is used to create the form and layouts
	setupUI();
	
	//in this program, the two major components are two group boxes
	//this first one handles the connection to BCI2000, and the second
	//provides info about the states being read.
	//Here we organize these in a vertical box layout, so that the connection
	//group appears above the state info group.
	mainLayout = new QVBoxLayout;
	mainLayout->addWidget(connectGroup,0);
	mainLayout->addWidget(statesGroup,0);
	setLayout(mainLayout);
	setWindowTitle("BCI Perturbation");
	reset();
	resize(250,800);
}

void mainUI::updateCfg()
{
	//this function reads in a configuration file, or creates it if it does not exist
}

//---------------------------------------------------------------
void mainUI::connectButCallback()
{
	//this SLOT is called when the connect button is clicked
	//the connections and sockets are first cleared and closed.
	//then the program attempts to connect to BCI2000 using the 
	//IP and ports specified. For a more "robust" example, see
	//the ConnectorFilter.cpp file in the BCI2000/src/Application/shared folder
	bool sendconnected = true, recconnected = true;
	stringstream str;

	recConnection.close();
	recConnection.clear();
	recSocket.close();
	
	sendConnection.close();
	sendConnection.clear();
	sendSocket.close();

	recSocket.open(receiveBox->text().toStdString().c_str());
	recConnection.open(recSocket);
	if (!recConnection.is_open())
	{
		//statusBox->setText("Could not connect to receiving address");
		sendconnected = false;
		str <<"Receive = false ";
	}
	else
	{
		str << "Reveive = true ";
	}

	sendSocket.open(sendBox->text().toStdString().c_str());
	sendConnection.open(sendSocket);
	if (!sendConnection.is_open())
	{
		//statusBox->setText("Could not connect to sending address");
		recconnected = false;
		str << "Send = false";
	}
	else
	{
		str << "Send = true";
	}
	
	statusBox->setText(str.str().c_str());
	if (!sendconnected || !recconnected)
		return;
	//else
	//	statusBox->setText("Connected!");

	//if we successfully connected, then start the timer
	timer->start();
}

void mainUI::timerUpdate()
{
	//this function is called every time the timer period is finished
	
	//start by stopping the timer, in case this function requires more time 
	//than the timer period to complete
	timer->stop();
	
	//updateVars();
	
	//try to read the states
	if (getStates())
	{
		//if we read the states, display them in the table
		map<string, int>::iterator mi;
		
		int count = 0;
		for (mi = states.begin(); mi != states.end(); mi++)
		{
			//str << mi.curr()->key << '\t' << mi.curr()->value << endl;
			if (stateTable->rowCount() < count+1)
				stateTable->setRowCount(count+1);
			
			stringstream value;
			value << mi->second;
			QTableWidgetItem *firstItem = new QTableWidgetItem(mi->first.c_str());
			QTableWidgetItem *secondItem = new QTableWidgetItem(value.str().c_str());
			stateTable->setItem(count,0, firstItem);
			stateTable->setItem(count,1, secondItem);
			count++;
		}
	}
	modCount++;
	//restart the timer
	timer->start();
}


//---------------------------------------------------------------
void mainUI::updateVars()
{
	/*
	if (addCheck->checkState() == Qt::Checked)
		addMod = true;
	else
		addMod = false;
	
	//triggerType = triggerCombo->currentText().toStdString();
	minDelay = atoi(minDelayBox->text().toStdString().c_str());
	maxDelay = atoi(maxDelayBox->text().toStdString().c_str());
	xMod = atoi(xModBox->text().toStdString().c_str());
	yMod = atoi(yModBox->text().toStdString().c_str());
	massInit = atoi(massInitBox->text().toStdString().c_str());
	FCinit = atoi(fcInitBox->text().toStdString().c_str());
	*/
}


//---------------------------------------------------------------
void mainUI::writeValue(std::string name, short value)
{
	//this function writes a value to a state in BCI2000
	//Isn't this easy?!
	sendConnection << name << ' ' << value << endl;
}

//---------------------------------------------------------------
void mainUI::getInitialValues()
{
}


//---------------------------------------------------------------
bool mainUI::getStates()
{
	//this function reads the state names and values from the TCP/UDP buffer
	int count = 0;
	while (recConnection.rdbuf()->in_avail())
	{
		string name;
		short value;
		stringstream str;
		//get the name and the value of the state
		recConnection >> name >> value;
		recConnection.ignore();
		
		if (!recConnection)
		{
			recConnection.clear();
		}
		count++;
		//if successful, add/update the state and value
		states[name] = value;
	}
	recConnection.clear();
	//if we got one or more states, return true
	if (count > 0)
		return true;
	
	//if we did not read any states, return false
	return false;
}

//----------------------------------------------
void mainUI::changeStateValue(int row, int column)
{
	//we only want to send a new value, not change the state name
	statusBox->setText("Changing value...");
	if (column == 0)
		return;
	bool ok1, ok2;
	
	if (!stateTable->item(row,0))
		return;
	
	string name = stateTable->item(row,0)->text().toStdString();
	
	stringstream str1;
	short value = stateTable->item(row, 1)->text().toShort(&ok1);
	
	str1 << "Enter a new value for " << name;

	short newValue = QInputDialog::getInteger(this, "",str1.str().c_str(),value, -32767, 32767, 1, &ok2 );
	
	if (ok1 && ok2)
	{
		writeValue(name, newValue);
		stringstream str;
		str << "Wrote " << name << ": "<< newValue<<endl;
		statusBox->setText(str.str().c_str());
	}
	else
		statusBox->setText("There was an error...");
		
}

//---------------------------------------------------------------
void mainUI::reset()
{
	//prevMass = -1;
	triggered = 0;
	prevITI = 0;
	noGetStates = 0;
	prevTrigger = 0;
	modCount = 0;
	initialized = false;
	prevFeedback = 0;
}



void mainUI::setupUI()
{		
	//this function sets up both group boxes, and sets the timer interval
	setupConnectionGroup();
	setupStatesGroup();
	resize(minimumSizeHint());
	
	timer = new QTimer(this);
	//here, the timer SIGNAL is connected to the timerUpdate SLOT
	timer->connect(timer, SIGNAL(timeout()), this, SLOT(timerUpdate()));
	timer->setInterval(100);
	timer->stop();	
}

void mainUI::setupConnectionGroup()
{
	//create the connection group box and setup the signals/slots
	connectGroup = new QGroupBox("Connection");
	//connectGroup->setGeometry(QRect(10, 180, 150, 150));
	
	connectLayout = new QVBoxLayout;

	receiveLabel = new QLabel("Input IP:Port");
	receiveBox = new QLineEdit();
	receiveBox->setText("localhost:20230");
	sendLabel = new QLabel("Output IP:Port");
	sendBox = new QLineEdit();
	sendBox->setText("localhost:20231");
	connectBut = new QPushButton("Connect");
	statusBox = new QLineEdit;
	connect(connectBut, SIGNAL(clicked()), this, SLOT(connectButCallback()));
	
	connectLayout->addWidget(receiveLabel,0);
	connectLayout->addWidget(receiveBox,0);
	connectLayout->addWidget(sendLabel,0);
	connectLayout->addWidget(sendBox,0);
	connectLayout->addWidget(connectBut,0,Qt::Alignment(4));
	connectLayout->addWidget(statusBox);
	
	connectGroup->setLayout(connectLayout);
}

void mainUI::setupStatesGroup()
{
	statesGroup = new QGroupBox("States");	
	//statesGroup->setGeometry(QRect(10, 10, 251, 171));

	statesLayout = new QGridLayout;
	statesLayout->setSpacing(6);
	statesLayout->setMargin(0);
	
	stateTable = new QTableWidget;
	stateTable->setColumnCount(2);
	stateTable->setRowCount(1);
	QStringList headers;
	headers << "State" << "Value";
	stateTable->setHorizontalHeaderLabels(headers);
	
	//add the signal/slot so that if a table value is double clicked, a dialog will appear
	stateTable->connect(stateTable, SIGNAL(cellDoubleClicked(int, int)), this, SLOT(changeStateValue(int, int)));
	
	statesLayout->addWidget(stateTable,0,0);
	
	statesGroup->setLayout(statesLayout);
}
