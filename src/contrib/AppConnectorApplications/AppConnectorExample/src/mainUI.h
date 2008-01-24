#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include <QtGui>
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <math.h>
#include <map>
#include <algorithm>

/* TCPstream is included from the BCI2000 src/shared folder. It was modified
slightly to work with linux/unix and gnu c++.
*/
#include "TCPStream.h"

//forward declare our qt classes to decrease compile time
class QGridLayout;
class QGroupBox;
class QHBoxLayout;
class QLabel;
class QLineEdit;
class QComboBox;
class QPushButton;
class QVBoxLayout;
class QCheckBox;
class QButtonGroup;
class QForm;
class QTableWidget;

using namespace std;

/*
Our main GUI class will be called mainUI. It inherits from the Qt
QWidget class, allowing it to use slots/signals, layouts, and
everything a form would need.
*/
class mainUI : public QWidget
{
	//this macro MUST be declared for any object which will use signals/slots
	//it provides the ability to use the private slots: class declaration
	
	Q_OBJECT
	
public:
	//the default constructor. In the implementation, this is where the form
	//generation will occur
	mainUI();
	
private slots:
	//these are our slots; if you are used to borland, these are basically
	//the callback functions for different actions, such as a button-click
	void connectButCallback();
	void timerUpdate();
	void changeStateValue(int row, int column);
	
private:
	//although Qt Designer allows us to graphically design our UI, it is helpful
	// to know what is going on in the actual code. Therefore, this class will manually
	// setup the forms rather than using the Qt Designer ui file.
	// layout ===========================================================
	QVBoxLayout *mainLayout;

	//connection objects
	QGroupBox *connectGroup;
	QVBoxLayout *connectLayout;
	QLabel *receiveLabel;
	QLineEdit *receiveBox;
	QLabel *sendLabel;
	QLineEdit *sendBox;
	QPushButton *connectBut;
	QLineEdit *statusBox;
	
	//perturbations objects
	QGroupBox *statesGroup;
	QGridLayout *statesLayout;
	QTableWidget *stateTable;
	
	//layout setup functions
	void setupUI();
	void setupConnectionGroup();
	void setupStatesGroup();
	
	// data members ====================================================
	string address;
	//these are the objects used to connect to bci2000
	receiving_udpsocket recSocket;
	tcpstream recConnection;
	sending_udpsocket sendSocket;
	tcpstream sendConnection;
	
	//this map holds the state names and associated values
	map<string, int> states;	
	
	//a timer to update the states
	QTimer *timer;
	
	int prevTrigger, prevITI, prevFeedback;
	string triggerType;
	bool addMod;
	int triggered;
	int minDelay, maxDelay, delay;
	short xMod, yMod;
	unsigned short modCount;
	short massInit, FCinit;
	bool initialized;
	
	int noGetStates;
	
	//more private helper functions
	void updateVars();
	void writeValue(string name, short value);
	void getInitialValues();
	void resetInitialValues();
	bool getStates();
	void reset();
	void updateCfg();
};
#endif
